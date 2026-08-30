# The Windows side of the now playing bridge.
#
# The desktop version of this is a shell script driving playerctl. This is the
# same idea for a system where the equivalent question is put to Windows: it
# asks what the machine is playing, leaves the answer in a file the game reads,
# and carries out whatever the game leaves in the orders file.
#
#     status|position|length|title|artist|album
#
# with the two times in microseconds, exactly as the desktop one writes it.
#
# It is a script rather than a program on purpose. The same questions asked
# from hand written interface declarations crashed on the first call, twice -
# the layouts involved are not something to be transcribed by hand. Asked this
# way the runtime does that part, which is also what makes seeking possible:
# the call that moves the playing position takes an argument, and getting one
# of those wrong is not something that can be recovered from.
#
# Started by the game with the folder to write in and the process to outlive.

param(
    [string] $Folder = ".",
    [int]    $Watch  = 0
)

$state  = Join-Path $Folder "retrocycles_media_state.txt"
$orders = Join-Path $Folder "retrocycles_media_cmd.txt"
$note   = Join-Path $Folder "retrocycles_media_log.txt"

function Write-Note( [string] $what ) {
    try {
        Add-Content -Path $note -Value ( "{0} {1}" -f (Get-Date -Format "HH:mm:ss"), $what )
    } catch { }
}

# Two of these at once take turns writing answers taken half a second apart,
# which makes the time in the file walk backwards. The second one leaves.
$only = New-Object System.Threading.Mutex( $false, "Local\retrocycles-media-bridge" )
if ( -not $only.WaitOne( 0 ) ) { exit 0 }

Write-Note "script started"

# These answer later. The runtime knows how to turn one into something that can
# be waited on; doing that by hand is the part that does not survive contact
# with a real machine.
try {
    Add-Type -AssemblyName System.Runtime.WindowsRuntime -ErrorAction Stop

    $asTask = ( [System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object {
        $_.Name -eq 'AsTask' -and
        $_.GetParameters().Count -eq 1 -and
        $_.GetParameters()[0].ParameterType.Name -eq 'IAsyncOperation`1'
    } )[0]

    if ( -not $asTask ) { throw "no way to wait on the system's answers" }
} catch {
    Write-Note ( "cannot reach the runtime: " + $_.Exception.Message )
    exit 2
}

function Wait-Answer( $operation, $type ) {
    try {
        $task = $asTask.MakeGenericMethod( $type ).Invoke( $null, @( $operation ) )
        if ( $task.Wait( 4000 ) ) { return $task.Result }
    } catch { }
    return $null
}

try {
    $managerType = [Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager,Windows.Media.Control,ContentType=WindowsRuntime]
    $propsType   = [Windows.Media.Control.GlobalSystemMediaTransportControlsSessionMediaProperties,Windows.Media.Control,ContentType=WindowsRuntime]
    $boolType    = [System.Boolean]

    $manager = Wait-Answer ( $managerType::RequestAsync() ) $managerType
    if ( -not $manager ) { throw "the session manager did not answer" }
} catch {
    Write-Note ( "no session manager: " + $_.Exception.Message )
    exit 3
}

Write-Note "session manager open"

function Clean( [string] $text ) {
    if ( -not $text ) { return "" }
    return ( $text -replace '[\|\r\n]', ' ' ).Trim()
}

#! Carries out one order. Everything but moving the position could be a
#! keypress, but the session takes them all and does not depend on which window
#! happens to be in front.
function Carry( $session, [string] $order ) {
    if ( -not $session ) { return }

    try {
        switch -Regex ( $order ) {
            '^prev$'       { Wait-Answer ( $session.TrySkipPreviousAsync() ) $boolType | Out-Null; return }
            '^next$'       { Wait-Answer ( $session.TrySkipNextAsync() )     $boolType | Out-Null; return }
            '^play-pause$' { Wait-Answer ( $session.TryTogglePlayPauseAsync() ) $boolType | Out-Null; return }
            '^seek'        {
                $seconds = [double]( $order.Substring( 4 ) )
                if ( $seconds -lt 0 ) { return }
                # the session counts in hundreds of nanoseconds
                $ticks = [long]( $seconds * 10000000 )
                Wait-Answer ( $session.TryChangePlaybackPositionAsync( $ticks ) ) $boolType | Out-Null
                return
            }
        }
    } catch {
        Write-Note ( "order '" + $order + "' refused: " + $_.Exception.Message )
    }
}

#! @return the line to leave for the game, empty when nothing is on.
function Look( $session ) {
    if ( -not $session ) { return "" }

    $props = Wait-Answer ( $session.TryGetMediaPropertiesAsync() ) $propsType
    if ( -not $props -or -not $props.Title ) { return "" }

    $playing = "Paused"
    try {
        if ( "$($session.GetPlaybackInfo().PlaybackStatus)" -eq "Playing" ) { $playing = "Playing" }
    } catch { }

    $position = 0
    $length   = 0
    try {
        $times = $session.GetTimelineProperties()
        # ticks are hundreds of nanoseconds where the game counts microseconds
        $position = [long]( ( $times.Position.Ticks - $times.StartTime.Ticks ) / 10 )
        $length   = [long]( ( $times.EndTime.Ticks  - $times.StartTime.Ticks ) / 10 )
        if ( $position -lt 0 ) { $position = 0 }
        if ( $length   -lt 0 ) { $length   = 0 }
    } catch { }

    return ( "{0}|{1}|{2}|{3}|{4}|{5}" -f $playing, $position, $length,
             ( Clean $props.Title ), ( Clean $props.Artist ), ( Clean $props.AlbumTitle ) )
}

$said = $false

while ( $true ) {
    if ( $Watch -gt 0 ) {
        if ( -not ( Get-Process -Id $Watch -ErrorAction SilentlyContinue ) ) { break }
    }

    $session = $null
    try { $session = $manager.GetCurrentSession() } catch { }

    # Read and clear in one go, so an order is carried out once.
    try {
        if ( Test-Path $orders ) {
            $order = ( Get-Content $orders -First 1 -ErrorAction SilentlyContinue )
            if ( $order ) {
                Clear-Content $orders -ErrorAction SilentlyContinue
                Carry $session $order.Trim()
            }
        }
    } catch { }

    try {
        $line = Look $session
        if ( $line ) {
            # Written aside and moved into place, so the game never reads half
            # a line. Rewritten every round even when nothing changed: the game
            # tells a paused player from a bridge that died by the file's age.
            # Written through the framework rather than with Set-Content:
            # that one puts a byte order mark in front, and the game compares
            # the first field of the line as it finds it.
            [System.IO.File]::WriteAllText( "$state.new", $line + [Environment]::NewLine,
                                            ( New-Object System.Text.UTF8Encoding $false ) )
            Move-Item -Path "$state.new" -Destination $state -Force

            if ( -not $said ) {
                Write-Note ( "first line: " + $line )
                $said = $true
            }
        }
    } catch { }

    Start-Sleep -Milliseconds 500
}
