//
//  SecondaryCamera.cpp
//  interface/src
//
//  Created by Samuel Gateau, Howard Stearns, and Zach Fox on 2017-06-08.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "SecondaryCamera.h"
#include <TextureCache.h>
#include <gpu/Context.h>

using RenderArgsPointer = std::shared_ptr<RenderArgs>;

void MainRenderTask::build(JobModel& task, const render::Varying& inputs, render::Varying& outputs, render::CullFunctor cullFunctor, bool isDeferred) {

    task.addJob<RenderShadowTask>("RenderShadowTask", cullFunctor);
    const auto items = task.addJob<RenderFetchCullSortTask>("FetchCullSort", cullFunctor);
    assert(items.canCast<RenderFetchCullSortTask::Output>());
    if (!isDeferred) {
        task.addJob<RenderForwardTask>("Forward", items);
    } else {
        task.addJob<RenderDeferredTask>("RenderDeferredTask", items);
    }
}

void SecondaryCameraRenderTaskConfig::resetSize(int width, int height) { // FIXME: Add an arg here for "destinationFramebuffer"
    bool wasEnabled = isEnabled();
    setEnabled(false);
    auto textureCache = DependencyManager::get<TextureCache>();
    textureCache->resetSpectatorCameraFramebuffer(width, height); // FIXME: Call the correct reset function based on the "destinationFramebuffer" arg
    setEnabled(wasEnabled);
}

void SecondaryCameraRenderTaskConfig::resetSizeSpectatorCamera(int width, int height) { // Carefully adjust the framebuffer / texture.
    resetSize(width, height);
}

class BeginSecondaryCameraFrame {  // Changes renderContext for our framebuffer and and view.
    glm::vec3 _position{};
    glm::quat _orientation{};
public:
    using Config = BeginSecondaryCameraFrameConfig;
    using JobModel = render::Job::ModelO<BeginSecondaryCameraFrame, RenderArgsPointer, Config>;
    BeginSecondaryCameraFrame() {
        _cachedArgsPointer = std::make_shared<RenderArgs>(_cachedArgs);
    }

    void configure(const Config& config) {
        if (config.enabled || config.alwaysEnabled) {
            _position = config.position;
            _orientation = config.orientation;
        }
    }

    void run(const render::RenderContextPointer& renderContext, RenderArgsPointer& cachedArgs) {
        auto args = renderContext->args;
        auto textureCache = DependencyManager::get<TextureCache>();
        gpu::FramebufferPointer destFramebuffer;
        destFramebuffer = textureCache->getSpectatorCameraFramebuffer(); // FIXME: Change the destination based on some unimplemented config var
        if (destFramebuffer) {
            // Caching/restoring the old values doesn't seem to be needed. Is it because we happen to be last in the pipeline (which would be a bug waiting to happen)?
            _cachedArgsPointer->_blitFramebuffer = args->_blitFramebuffer;
            _cachedArgsPointer->_viewport = args->_viewport;
            _cachedArgsPointer->_displayMode = args->_displayMode;
            args->_blitFramebuffer = destFramebuffer;
            args->_viewport = glm::ivec4(0, 0, destFramebuffer->getWidth(), destFramebuffer->getHeight());
            args->_displayMode = RenderArgs::MONO;

            gpu::doInBatch(args->_context, [&](gpu::Batch& batch) {
                batch.disableContextStereo();
            });

            auto srcViewFrustum = args->getViewFrustum();
            srcViewFrustum.setPosition(_position);
            srcViewFrustum.setOrientation(_orientation);
            args->pushViewFrustum(srcViewFrustum);
            cachedArgs = _cachedArgsPointer;
        }
    }

protected:
    RenderArgs _cachedArgs;
    RenderArgsPointer _cachedArgsPointer;
};

class EndSecondaryCameraFrame {  // Restores renderContext.
public:
    using JobModel = render::Job::ModelI<EndSecondaryCameraFrame, RenderArgsPointer>;

    void run(const render::RenderContextPointer& renderContext, const RenderArgsPointer& cachedArgs) {
        auto args = renderContext->args;
        args->_blitFramebuffer = cachedArgs->_blitFramebuffer;
        args->_viewport = cachedArgs->_viewport;
        args->popViewFrustum();
        args->_displayMode = cachedArgs->_displayMode;

        gpu::doInBatch(args->_context, [&](gpu::Batch& batch) {
            batch.restoreContextStereo();
        });
    }
};

void SecondaryCameraRenderTask::build(JobModel& task, const render::Varying& inputs, render::Varying& outputs, render::CullFunctor cullFunctor) {
    const auto cachedArg = task.addJob<BeginSecondaryCameraFrame>("BeginSecondaryCamera");
    const auto items = task.addJob<RenderFetchCullSortTask>("FetchCullSort", cullFunctor);
    assert(items.canCast<RenderFetchCullSortTask::Output>());
    task.addJob<RenderDeferredTask>("RenderDeferredTask", items);
    task.addJob<EndSecondaryCameraFrame>("EndSecondaryCamera", cachedArg);
}