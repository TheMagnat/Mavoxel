
#pragma once

#include <GraphicObjects/DrawableSingle.hpp>


namespace mav {

    enum class SceneType {
        RAY_TRACING,
        FILTER
    };

    struct DrawableInformation {

        DrawableInformation(mav::DrawableSingle* drawable_p, int rendererInIndex_p, int rendererIndex_p, SceneType sceneType_p, std::optional<glm::uvec2> outputResolution_p) :
            drawable(drawable_p), rendererInIndex(rendererInIndex_p), rendererIndex(rendererIndex_p), sceneType(sceneType_p), outputResolution(outputResolution_p) {}

        mav::DrawableSingle* drawable;
        int rendererInIndex; //Renderer input
        int rendererIndex; //Renderer output
        SceneType sceneType;
        std::optional<glm::uvec2> outputResolution;
    };

}
