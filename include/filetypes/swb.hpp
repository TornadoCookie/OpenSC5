#ifndef _SWB_H_
#define _SWB_H_

#include <vector>
#include <raylib.h>
#include <stdint.h>

struct SwarmParticleEffectWiggle {
    float timeRate;
    Vector3 rateDirection;
    Vector3 wiggleDirection;
};

struct SwarmParticleEffectSurface {
    int flags;
    uint64_t surfaceMapID;
    float bounce;
    float slide;
    float collisionRadius;
    float deathProbability;
    int pinOffset;
    std::vector<Vector3> surfacePoints;
};

struct SwarmParticleEffectPathPoint {
    Vector3 position;
    Vector3 velocity;
    float time;
};

struct SwarmParticleEffect {
    int version;
    int flags;
    float lifetime[2];
    float prerollTime;
    float emitDelay[2];
    float emitRetrigger[2];
    BoundingBox emitDirection;
    float emitSpeed[2];
    BoundingBox emitVolume;
    float emitTorusWidth;
    std::vector<float> emitRateCurve;
    float emitRateCurveTime;
    int emitRateCurveCycles;
    float emitRateSpeedScale;
    std::vector<float> sizeCurve;
    float sizeVary;
    std::vector<float> aspectRatioCurve;
    float aspectRatioVary;
    float rotationVary;
    float rotationOffset;
    std::vector<float> rotationCurve;
    // alphaCurve and alphaVary are in below
    std::vector<Color> colorCurve;
    Color colorVary;

    uint64_t materialId;
    int format;
    int drawMode;
    int buffer;
    int layer;
    float sortOffset;
    int physicsType;
    int overrideSet;
    int tileCountU;
    int tileCountV;
    int alignMode;
    float frameSpeed;
    int frameStart;
    int frameCount;
    int frameRandom;
    Vector3 directionalForcesSum;
    float windStrength;
    float gravityStrength;
    float radialForce;
    Vector3 radialForceLocation;
    float drag;
    float velocityStretch;
    float screwRate;
    std::vector<SwarmParticleEffectWiggle> wiggles;

    // screen bloom alpha rate
    // screen bloom alpha base
    // screen bloom size rate
    // screen bloom size base

    std::vector<Color> loopBoxColorCurve;
    std::vector<SwarmParticleEffectSurface> surfaces;

    float mapBounce;
    float mapRepulseHeight;
    float mapRepulseStrength;
    float mapRepulseScoutDistance;
    float mapRepulseVertical;
    float mapRepulseKillHeight;
    float probabilityDeath;
    float altitudeRange[2];
    //uint64_t forceMap;
    //uint64_t emitRateMap;
    //uint64_t emitColorMap;

    // random walk
    float time[2];
    float strength[2];
    float turnRange;
    float turnOffset;
    float mix;
    std::vector<float> turnOffsetCurve;
    int walkLoopType;

    Vector3 attractorOrigin;
    std::vector<float> attractorStrengthCurve;

    float range;
    float killRange;

    std::vector<SwarmParticleEffectPathPoint> pathPoints;
};

struct Swarm {
    int version;
    std::vector<SwarmParticleEffect> particleEffects;
};

#endif
