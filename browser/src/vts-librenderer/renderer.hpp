/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RENDERER_HPP_deh4f6d4hj
#define RENDERER_HPP_deh4f6d4hj

#include <unordered_map>

#include <vts-browser/log.hpp>
#include <vts-browser/math.hpp>
#include <vts-browser/buffer.hpp>

#include "include/vts-renderer/classes.hpp"
#include "include/vts-renderer/renderer.hpp"

namespace vts
{

class CameraDraws;
class DrawSurfaceTask;
class DrawSimpleTask;

namespace renderer
{

class RenderContextImpl;
class GeodataBase;
struct Text;

// reading depth immediately requires implicit sync between cpu and gpu, which is wasteful
//   instead, this buffer stores the depth into temporary pbo (pixel buffer object)
//   and reads it to cpu in separate step (in next frame)
class DepthBuffer
{
private:
    static const uint32 PboCount = 2;
    Buffer buffer;
    mat4 conv[PboCount];
    uint32 w[PboCount], h[PboCount];
    uint32 pbo[PboCount];
    uint32 tw, th;
    uint32 fbo, tex;
    uint32 index;

    double valuePix(uint32 x, uint32 y);

public:
    DepthBuffer();
    ~DepthBuffer();

    const mat4 &getConv() const;

    void performCopy(uint32 sourceTexture, uint32 w, uint32 h,
        const mat4 &storeConv);

    // xy in -1..1
    // returns 0..1 in logarithmic depth
    double value(double x, double y);

    std::shared_ptr<Shader> shaderCopyDepth;
    std::shared_ptr<Mesh> meshQuad;
};

class ShaderAtm : public Shader
{
public:
    struct AtmBlock
    {
        mat4f uniAtmViewInv;
        vec4f uniAtmColorHorizon;
        vec4f uniAtmColorZenith;
        vec4f uniAtmSizes; // atmosphere thickness (divided by major axis), major / minor axes ratio, inverze major axis
        vec4f uniAtmCoefs; // horizontal exponent, colorGradientExponent
        vec3f uniAtmCameraPosition; // world position of camera (divided by major axis)
    };

    void initializeAtmosphere();
};

struct Rect
{
    vec2f a, b;
    Rect();
    bool valid() const;
    static bool overlaps(const Rect &a, const Rect &b);
};

struct GeodataJob
{
    Rect rect; // ndc space
    std::shared_ptr<GeodataBase> g;
    uint32 itemIndex; // -1 means all
    float importance;
    float opacity; // 1 .. 0
    float stick; // stick length (pixels)
    float ndcZ;
    GeodataJob(const std::shared_ptr<GeodataBase> &g, uint32 itemIndex);
};

extern uint32 maxAntialiasingSamples;
extern float maxAnisotropySamples;

void clearGlState();
void enableClipDistance(bool enable);

class RenderViewImpl
{
public:
    Camera *const camera;
    RenderView *const api;
    RenderContextImpl *const context;

    RenderVariables vars;
    RenderOptions options;
    DepthBuffer depthBuffer;
    std::vector<GeodataJob> geodataJobs;
    std::unordered_map<std::string, GeodataJob> hysteresisJobs;
    std::shared_ptr<UniformBuffer> uboAtm;
    std::shared_ptr<UniformBuffer> uboGeodataCamera;
    std::vector<std::unique_ptr<UniformBuffer>> uboCacheVector;
    uint32 uboCacheIndex;
    CameraDraws *draws;
    const MapCelestialBody *body;
    Texture *atmosphereDensityTexture;
    GeodataBase *lastUboViewPointer;
    mat4 view;
    mat4 viewInv;
    mat4 proj;
    mat4 projInv;
    mat4 viewProj;
    mat4 viewProjInv;
    mat4 davidProj;
    mat4 davidProjInv;
    vec3 zBufferOffsetValues;
    uint32 width;
    uint32 height;
    uint32 antialiasingPrev;
    double elapsedTime;
    bool projected;

    RenderViewImpl(Camera *camera, RenderView *api,
        RenderContextImpl *context);
    UniformBuffer *getUbo();
    void drawSurface(const DrawSurfaceTask &t);
    void drawInfographic(const DrawSimpleTask &t);
    void updateFramebuffers();
    void updateAtmosphereBuffer();
    void getWorldPosition(const double screenPos[2], double worldPos[3]);
    void renderCompass(const double screenPosSize[3],
        const double mapRotation[3]);
    void renderValid();
    void renderEntry();

    bool geodataTestVisibility(
        const float visibility[4],
        const vec3 &pos, const vec3f &up);
    bool geodataDepthVisibility(const vec3 &pos, float threshold);
    mat4 depthOffsetCorrection(const std::shared_ptr<GeodataBase> &g) const;
    void renderGeodataQuad(const Rect &rect, float depth, const vec4f &color);
    void bindUboView(const std::shared_ptr<GeodataBase> &gg);
    void computeZBufferOffsetValues();
    void bindUboCamera();
    void renderGeodata();
    void generateJobs();
    void sortJobsByZIndexAndImportance();
    void renderJobMargins();
    void filterJobs();
    void processHysteresisJobs();
    void sortJobsByZIndexAndDepth();
    void renderStick(const GeodataJob &job, const vec3 &worldPosition);
    void renderIcon(const GeodataJob &job, const vec3 &worldPosition);
    void renderJobs();
};

class RenderContextImpl
{
public:
    RenderContext *const api;

    std::shared_ptr<Texture> texCompas;
    std::shared_ptr<ShaderAtm> shaderSurface;
    std::shared_ptr<ShaderAtm> shaderBackground;
    std::shared_ptr<Shader> shaderInfographics;
    std::shared_ptr<Shader> shaderTexture;
    std::shared_ptr<Shader> shaderCopyDepth;
    std::shared_ptr<Shader> shaderGeodataColor;
    std::shared_ptr<Shader> shaderGeodataLine;
    std::shared_ptr<Shader> shaderGeodataPoint;
    std::shared_ptr<Shader> shaderGeodataPointLabel;
    std::shared_ptr<Shader> shaderGeodataTriangle;
    std::shared_ptr<Mesh> meshQuad; // positions: -1 .. 1
    std::shared_ptr<Mesh> meshRect; // positions: 0 .. 1
    std::shared_ptr<Mesh> meshLine;
    std::shared_ptr<Mesh> meshEmpty;

    RenderContextImpl(RenderContext *api);
    ~RenderContextImpl();
};

} // namespace renderer

} // namespace vts

#endif
