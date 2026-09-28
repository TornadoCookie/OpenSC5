#include <stdio.h>
#include <raylib.h>
#include <stdint.h>
#include <stdlib.h>
#include <cpl_endian.h>

#include "filetypes/swb.hpp"
#include "memstream.h"

SwarmParticleEffect LoadSwarmParticleEffect(MemStream *stm, int version)
{
    SwarmParticleEffect particleEffect;

    printf("at %d/%d\n", (long)stm->cur - (long)stm->buf, stm->size);

    particleEffect.version = version;
    particleEffect.flags = memstream_readb32(stm);
    particleEffect.lifetime[0] = memstream_readbfloat(stm);
    particleEffect.lifetime[1] = memstream_readbfloat(stm);
    particleEffect.prerollTime = memstream_readbfloat(stm);
    particleEffect.emitDelay[0] = memstream_readbfloat(stm);
    particleEffect.emitDelay[1] = memstream_readbfloat(stm);
    particleEffect.emitRetrigger[0] = memstream_readbfloat(stm);
    particleEffect.emitRetrigger[1] = memstream_readbfloat(stm);
    particleEffect.emitDirection = memstream_readbbox(stm); // be
    particleEffect.emitSpeed[0] = memstream_readbfloat(stm);
    particleEffect.emitSpeed[1] = memstream_readbfloat(stm);
    particleEffect.emitVolume = memstream_readbbox(stm);
    particleEffect.emitTorusWidth = memstream_readbfloat(stm);

    int emitRateCurveCount = memstream_readb32(stm);
    printf("emit rate curve count %d\n", emitRateCurveCount);
    for (int i = 0; i < emitRateCurveCount; i++)
    {
        particleEffect.emitRateCurve.push_back(memstream_readbfloat(stm));
    }

    particleEffect.emitRateCurveTime = memstream_readbfloat(stm);
    particleEffect.emitRateCurveCycles = memstream_readb16(stm);
    particleEffect.emitRateSpeedScale = memstream_readbfloat(stm);

    memstream_readfloat(stm); // ???
    memstream_readfloat(stm); // ???

    int sizeCurveCount = memstream_readb32(stm);
    for (int i = 0; i < sizeCurveCount; i++)
    {
        particleEffect.sizeCurve.push_back(memstream_readbfloat(stm));
    }

    //particleEffect.sizeVary = memstream_readbfloat(stm);
    
    int aspectRatioCurveCount = memstream_readb32(stm);
    for (int i = 0; i < aspectRatioCurveCount; i++)
    {
        particleEffect.aspectRatioCurve.push_back(memstream_readbfloat(stm));
    }

    particleEffect.aspectRatioVary = memstream_readbfloat(stm);
    particleEffect.rotationVary = memstream_readbfloat(stm);
    particleEffect.rotationOffset = memstream_readbfloat(stm);

    int rotationCurveCount = memstream_readb32(stm);
    for (int i = 0; i < rotationCurveCount; i++)
    {
        particleEffect.rotationCurve.push_back(memstream_readbfloat(stm));
    }

    int alphaCurveCount = memstream_readb32(stm);
    for (int i = 0; i < alphaCurveCount; i++) memstream_readbfloat(stm); // SKIP

    memstream_readbfloat(stm); // alpha vary

    int colorCurveCount = memstream_readb32(stm);
    for (int i = 0; i < colorCurveCount; i++)
    {
        particleEffect.colorCurve.push_back(memstream_readc3(stm));
    }

    particleEffect.colorVary = memstream_readc3(stm);

    particleEffect.materialId = memstream_readb64(stm);
    particleEffect.format = memstream_read8(stm);
    particleEffect.drawMode = memstream_read8(stm);

    if ((particleEffect.drawMode & 0x80) == 0x80) memstream_readb32(stm);
    if ((particleEffect.drawMode & 0x40) == 0x40) memstream_read8(stm);

    if (version >= 6) memstream_readb16(stm);
    else memstream_read8(stm);

    particleEffect.buffer = memstream_read8(stm);
    particleEffect.layer = memstream_readb16(stm);
    particleEffect.sortOffset = memstream_readbfloat(stm);
    memstream_read64(stm); // second iid

    particleEffect.physicsType = memstream_read8(stm);
    particleEffect.overrideSet = memstream_read8(stm);
    particleEffect.tileCountU = memstream_read8(stm);
    particleEffect.tileCountV = memstream_read8(stm);
    particleEffect.alignMode = memstream_read8(stm);
    particleEffect.frameSpeed = memstream_readbfloat(stm);
    particleEffect.frameStart = memstream_read8(stm);
    particleEffect.frameCount = memstream_read8(stm);
    particleEffect.frameRandom = memstream_read8(stm);
    particleEffect.directionalForcesSum = memstream_readv3(stm);
    particleEffect.windStrength = memstream_readbfloat(stm);
    particleEffect.gravityStrength = memstream_readbfloat(stm);
    particleEffect.radialForce = memstream_readbfloat(stm);
    particleEffect.radialForceLocation = memstream_readv3(stm);
    particleEffect.drag = memstream_readbfloat(stm);
    particleEffect.velocityStretch = memstream_readbfloat(stm);
    particleEffect.screwRate = memstream_readbfloat(stm);

    int wiggleCount = memstream_readb32(stm);
    for (int i = 0; i < wiggleCount; i++)
    {
        SwarmParticleEffectWiggle wiggle;
        wiggle.timeRate = memstream_readbfloat(stm);
        wiggle.rateDirection = memstream_readv3(stm);
        wiggle.wiggleDirection = memstream_readv3(stm);
        particleEffect.wiggles.push_back(wiggle);
    }

    memstream_read32(stm); // screen bloom

    int loopBoxColorCurveCount = memstream_readb32(stm);
    for (int i = 0; i < loopBoxColorCurveCount; i++)
    {
        particleEffect.loopBoxColorCurve.push_back(memstream_readc3(stm));
    }

    int loopBoxAlphaCurveCount = memstream_readb32(stm);
    for (int i = 0; i < loopBoxAlphaCurveCount; i++) memstream_readbfloat(stm); // skip

    int surfaceCount = memstream_readb32(stm);
    for (int i = 0; i < surfaceCount; i++) // TODO
    {
        SwarmParticleEffectSurface surface;
        surface.flags = memstream_readb32(stm);
        surface.surfaceMapID = memstream_readb64(stm);
        surface.bounce = memstream_readbfloat(stm);
        surface.slide = memstream_readbfloat(stm);
        surface.collisionRadius = memstream_readbfloat(stm);
        surface.deathProbability = memstream_readbfloat(stm);
        surface.pinOffset = memstream_readb32(stm);
        // what is a zstring????
        printf("zstring at %#X\n", (long)stm->cur - (long)stm->buf);
        exit(EXIT_SUCCESS);
    }

    particleEffect.mapBounce = memstream_readbfloat(stm);
    particleEffect.mapRepulseHeight = memstream_readbfloat(stm);
    particleEffect.mapRepulseStrength = memstream_readbfloat(stm);
    particleEffect.mapRepulseScoutDistance = memstream_readbfloat(stm);
    particleEffect.mapRepulseVertical = memstream_readbfloat(stm);
    // start skip.
    memstream_skip(stm, 40);
    particleEffect.mapRepulseKillHeight = memstream_readbfloat(stm);
    particleEffect.probabilityDeath = memstream_readbfloat(stm);
    particleEffect.altitudeRange[0] = memstream_readfloat(stm);
    particleEffect.altitudeRange[1] = memstream_readfloat(stm);
    memstream_read64(stm);
    memstream_read64(stm);
    memstream_read64(stm);

    // random walk
    particleEffect.time[0] = memstream_readbfloat(stm);
    particleEffect.time[1] = memstream_readbfloat(stm);
    particleEffect.strength[0] = memstream_readbfloat(stm);
    particleEffect.strength[1] = memstream_readbfloat(stm);
    particleEffect.turnRange = memstream_readbfloat(stm);
    particleEffect.turnOffset = memstream_readbfloat(stm);
    particleEffect.mix = memstream_readbfloat(stm);
    int turnOffsetCurveCount = memstream_readb32(stm);
    for (int i = 0; i < turnOffsetCurveCount; i++)
    {
        particleEffect.turnOffsetCurve.push_back(memstream_readbfloat(stm));
    }
    particleEffect.walkLoopType = memstream_read8(stm);

    particleEffect.attractorOrigin = memstream_readv3(stm);

    int attractorStrengthCurveCount = memstream_readb32(stm);
    for (int i = 0; i < attractorStrengthCurveCount; i++)
    {
        particleEffect.attractorStrengthCurve.push_back(memstream_readbfloat(stm));
    }
    particleEffect.range = memstream_readbfloat(stm);
    particleEffect.killRange = memstream_readbfloat(stm);

    int pathPointCount = memstream_readb32(stm);
    for (int i = 0; i < pathPointCount; i++)
    {
        SwarmParticleEffectPathPoint pathPoint;
        pathPoint.position = memstream_readv3(stm);
        pathPoint.velocity = memstream_readv3(stm);
        pathPoint.time = memstream_readbfloat(stm);
        particleEffect.pathPoints.push_back(pathPoint);
    }

    if (version >= 2)
    {
        memstream_skip(stm, sizeof(float)*3);
        int count = memstream_readb32(stm);
        memstream_skip(stm, count*sizeof(float)*3);
    }

    if (version >= 3) memstream_skip(stm, 1);
    if (version >= 4) memstream_skip(stm, 4);
    if (version >= 5) memstream_skip(stm, 4);

    return particleEffect;
}

Swarm LoadSwarm(unsigned char *data, int dataSize)
{
    Swarm swarm;
    MemStream stm = memstream_read_create(data, dataSize);

    swarm.version = memstream_readb16(&stm);

    for (;;)
    {
        int blockType = memstream_readb16(&stm);
        int version = memstream_readb16(&stm);
        int count = memstream_readb32(&stm);
        if (blockType == 0xFFFF) break;
        switch (blockType)
        {
            case 0x0001: // particle effect
            {
                swarm.particleEffects.reserve(count);
                for (int i = 0; i < count; i++)
                {
                    swarm.particleEffects.push_back(LoadSwarmParticleEffect(&stm, version));
                }
            } break;
            default:
            {
                printf("Unknown BlockType %#X.\n", blockType);
                exit(EXIT_SUCCESS);
            } break;
        }
    }

    printf("made it\n");

    return swarm;
}

int main(int argc, char ** argv)
{
    int dataSize = 0;
    unsigned char *data = LoadFileData(argv[1], &dataSize);
    
    LoadSwarm(data, dataSize);

    return 0;
}
