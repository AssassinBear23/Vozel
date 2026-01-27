#pragma once

#include "../panel.h"

namespace editor
{
    class projectPanel : public Panel
    {
    public:
        projectPanel() : Panel("Project", true) {}
        void draw(EditorContext& ctx) override;
    };
} // namespace editor