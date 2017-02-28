#ifndef MAPRESOURCES_H_hdsgfhjuwebgfj
#define MAPRESOURCES_H_hdsgfhjuwebgfj

#include <memory>
#include <string>

#include "../../vts-libs/vts/metatile.hpp"

#include <renderer/foundation.h>
#include <renderer/resource.h>
#include <renderer/gpuResources.h>
#include "math.h"

namespace melown
{
    class MetaTile : public Resource, public vadstena::vts::MetaTile
    {
    public:
        MetaTile(const std::string &name);
        void load(class MapImpl *base) override;
    };

    class MeshPart
    {
    public:
        std::shared_ptr<GpuMeshRenderable> renderable;
        mat4 normToPhys;
    };

    class MeshAggregate : public Resource
    {
    public:
        MeshAggregate(const std::string &name);

        void load(class MapImpl *base) override;

        std::vector<MeshPart> submeshes;
    };

    class ExternalBoundLayer : public Resource
    {
    public:
        ExternalBoundLayer(const std::string &name);

        void load(class MapImpl *base) override;

        vadstena::registry::BoundLayer bl;
    };
}

#endif
