

#ifndef ArmageTron_CYCLE_H
#define ArmageTron_CYCLE_H



#include "gStuff.h"



#include "eNetGameObject.h"
#include "tList.h"
#include "nObserver.h"
#include "rDisplayList.h"

#include "gCycleMovement.h"

class rModel;
class gTextureCycle;
class eSoundPlayer;
class gSensor;
class gNetPlayerWall;
class gPlayerWall;
class eTempEdge;
struct gPredictPositionData;


extern REAL sg_delayCycle;


extern bool headlights;


extern REAL sg_rubberCycle;





class gCycleMemoryEntry;

class gCycleMemory{
    friend class gCycleMemoryEntry;

    tList<gCycleMemoryEntry>     memory;  

public:
    
    gCycleMemoryEntry* Remember(const gCycle *cycle);
    int Len() const {return memory.Len();}
    gCycleMemoryEntry* operator() (int i)  const;
    gCycleMemoryEntry* Latest   (int side)  const;
    gCycleMemoryEntry* Earliest (int side)  const;

    void Clear();

    gCycleMemory();
    ~gCycleMemory();
};


class gCycleExtrapolator: public gCycleMovement
{
public:
    void CopyFrom( const gCycleMovement& other );							
    void CopyFrom( const SyncData& sync, const gCycle& other );	        	

    gCycleExtrapolator(eGrid *grid, const eCoord &pos,const eCoord &dir,ePlayerNetID *p=NULL,bool autodelete=1);
    
    virtual ~gCycleExtrapolator();

    

    virtual bool EdgeIsDangerous(const eWall *w, REAL time, REAL a) const;

    virtual void PassEdge(const eWall *w,REAL time,REAL a,int recursion=1);

    virtual bool TimestepCore(REAL currentTime, bool calculateAcceleration = true );

    

    REAL			  trueDistance_;										
private:
    

    virtual nDescriptor& CreatorDescriptor() const;

    const gCycleMovement* parent_;												
};

class gCycleChatBot;

#ifndef DEDICATED
class gCycleWallsDisplayListManager
{
    friend class gNetPlayerWall;

public:
    gCycleWallsDisplayListManager();
    ~gCycleWallsDisplayListManager();

    
    static bool CannotHaveList( REAL distance, gCycle const * cycle );

    
    void RenderAllWithDisplayList( eCamera const * camera, gCycle * cycle );

    
    static void RenderAll( eCamera const * camera, gCycle * cycle, gNetPlayerWall * list );

    
    void RenderAll( eCamera const * camera, gCycle * cycle );
    bool Walls() const
    {
        return wallList_ || wallsWithDisplayList_;
    }

    void Clear( int inhibit = 0 )
    {
        displayList_.Clear( inhibit );
    }
private:
    gNetPlayerWall *                wallList_;                      
    gNetPlayerWall *                wallsWithDisplayList_;          
    rDisplayList                    displayList_;                   
    REAL                            wallsWithDisplayListMinDistance_; 
    int                             wallsInDisplayList_;            
};
#endif


class gCycle: public gCycleMovement
{
    friend class DemoPlayerManager;
    friend class gPlayerWall;
    friend class gNetPlayerWall;
    friend class gDestination;
    friend class gCycleWallRenderer;

    eSoundPlayer *engine;
    eSoundPlayer *turning;
    eSoundPlayer *spark;

    REAL spawnTime_;    
    REAL lastTimeAnim;  
    REAL timeCameIntoView;

    friend class gCycleChatBot;
    std::unique_ptr< gCycleChatBot > chatBot_;

    bool dropWallRequested_; 
public:
    
    REAL modPeakRubberInSpot;
    eCoord modSpotPosition;
    bool modWasUsingRubber;
    REAL modLastRubberAmount;
    REAL modLastWaypointTime;
    REAL modRubberAtEventStart;

    eCoord            lastGoodPosition_;    

    REAL skew,skewDot;						

    bool 				mp; 				

    rModel *body,*front,*rear,
    *customModel;

    gTextureCycle  *wheelTex,*bodyTex;
    gTextureCycle  *customTexture;

    eCoord rotationFrontWheel,rotationRearWheel; 	
    REAL   heightFrontWheel,heightRearWheel;  		
public:
    

    static uActionPlayer s_brake;
    static uActionPlayer se_perfectTurnLeft;
    static uActionPlayer se_perfectTurnRight;
    static uActionPlayer se_autoEscapeLeft;
    static uActionPlayer se_autoEscapeRight;
    gCycleMemory memory;

    
    bool intentTurnLeft;
    bool intentTurnRight;
    
    
    int intentAutoEscapeDir;
    REAL previousFrameRubber;

    
    REAL rightTurnTimes[3];
    REAL leftTurnTimes[3];
    int rightTurnCount;
    int leftTurnCount;

    
    int debug_GapTimerTicks;      
    int debug_lastTurnDir;        

    gRealColor color_;
    gRealColor trailColor_;
    float lastTexR_, lastTexG_, lastTexB_;
    double lastTexUpdateTime_;

    
    
    
    eCoord correctPosSmooth;
    eCoord predictPosition_; 

    
    REAL correctDistanceSmooth;

private:
    void TransferPositionCorrectionToDistanceCorrection();

#ifndef DEDICATED
    gCycleWallsDisplayListManager displayList_;                     
#endif

    tCHECKED_PTR(gNetPlayerWall)	currentWall;                    
    tCHECKED_PTR(gNetPlayerWall)	lastWall;                       
    tCHECKED_PTR(gNetPlayerWall)	lastNetWall;                    

    
    SyncData									lastSyncMessage_;	
    tJUST_CONTROLLED_PTR<gCycleExtrapolator>	extrapolator_;		
    bool										resimulate_;		

    void	ResetExtrapolator();							
    bool	Extrapolate( REAL dt );							
    void	SyncFromExtrapolator();							

    virtual void OnNotifyNewDestination(gDestination *dest);   
    virtual void OnDropTempWall        ( gPlayerWall * wall, eCoord const & position, eCoord const & direction );   

    

    nTimeRolling nextSync, nextSyncOwner;
    REAL lastSyncOwnerGameTime_;    

    void MyInitAfterCreation();

    void SetCurrentWall(gNetPlayerWall *w);

    void PreparePredictPosition( gPredictPositionData & data ); 
    REAL CalculatePredictPosition( gPredictPositionData & data ); 
protected:
    virtual ~gCycle();

    virtual void OnRemoveFromGame(); 

    virtual void OnRoundEnd();    

    
public:
    virtual void Die ( REAL time )  ;  
    void KillAt( const eCoord& pos );  

    int WindingNumber() const {return windingNumber_;}

    virtual bool            Vulnerable              ()                                    const     ;   

    

    virtual void InitAfterCreation();
    gCycle(eGrid *grid, const eCoord &pos,const eCoord &dir,ePlayerNetID *p=NULL);

    static	void 	SetWallsStayUpDelay		( REAL delay );				
    static	void 	SetWallsLength			( REAL length);				
    static	void 	SetExplosionRadius		( REAL radius);				

    static	REAL 	WallsStayUpDelay()	 { return wallsStayUpDelay;	}	
    static	REAL	WallsLength()	 	 { return wallsLength;		}	
    REAL	        MaxWallsLength() const;                             
    REAL	        ThisWallsLength() const;                            
    REAL	        WallEndSpeed() const;                               
    static	REAL	ExplosionRadius()	 { return explosionRadius;	}	

    bool    IsMe( eGameObject const * other ) const;              

    
    gCycle(nMessage &m);
    virtual void WriteCreate(nMessage &m);
    virtual void WriteSync(nMessage &m);
    virtual void ReadSync(nMessage &m);
    virtual void RequestSyncOwner(); 
    virtual void RequestSyncAll(); 

    virtual void SyncEnemy ( const eCoord& begWall );    
    

    virtual void ReceiveControl(REAL time,uActionPlayer *Act,REAL x);
    virtual void PrintName(tString &s) const;
    virtual bool ActionOnQuit();

    virtual nDescriptor &CreatorDescriptor() const;
    virtual bool SyncIsNew(nMessage &m);
    

    virtual bool Timestep(REAL currentTime);
    virtual bool TimestepCore(REAL currentTime,bool calculateAcceleration = true);

    virtual void InteractWith(eGameObject *target,REAL time,int recursion=1);

    virtual bool EdgeIsDangerous(const eWall *w, REAL time, REAL a) const;

    virtual void PassEdge(const eWall *w,REAL time,REAL a,int recursion=1);

    virtual REAL PathfindingModifier( const eWall *w ) const;

    virtual bool Act(uActionPlayer *Act,REAL x);

    virtual bool DoTurn(int dir);
    void DropWall( bool buildNew=true );                                    

    

    virtual void Kill();

    const eTempEdge* Edge();
    const gPlayerWall* CurrentWall();
    

#ifndef DEDICATED
    void UpdateTexturesColor();
    virtual void Render(const eCamera *cam);

    virtual void RenderName( const eCamera *cam );

    virtual bool RenderCockpitFixedBefore(bool primary=true);

    virtual void SoundMix(unsigned char *dest,unsigned int len,
                          int viewer,REAL rvol,REAL lvol);
#endif

    virtual eCoord CamPos() const;
    virtual eCoord PredictPosition() const;
    virtual eCoord  CamTop() const;

    virtual void RightBeforeDeath( int numTries );

#ifdef POWERPAK_DEB
    virtual void PPDisplay();
#endif

    static 	void	PrivateSettings();									

    
    

private:
    static	REAL	wallsStayUpDelay;			
    static	REAL	wallsLength;				
    static	REAL	explosionRadius;			

protected:
    virtual 	bool 			DoIsDestinationUsed		( const gDestination *	dest		) const		;	
};

#endif

