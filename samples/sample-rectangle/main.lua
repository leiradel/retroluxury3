local app = require 'sokol.app'
local gfx = require 'sokol.gfx'
local gp = require 'sokol.gp'

-- Called on every frame of the application.
local function frame()
    -- Get current window size.
    local width, height = app.width(), app.height()
    local ratio = width / height

    -- Begin recording draw commands for a frame buffer of size (width, height).
    gp.begin(width, height)
    -- Set frame buffer drawing region to (0,0,width,height).
    gp.viewport(0, 0, width, height)
    -- Set drawing coordinate space to (left=-ratio, right=ratio, top=1, bottom=-1).
    gp.project(-ratio, ratio, 1.0, -1.0)

    -- Clear the frame buffer.
    gp.set_color(0.1, 0.1, 0.1, 1.0)
    gp.clear()

    -- Draw an animated rectangle that rotates and changes its colors.
    local time = app.frame_count() * app.frame_duration()
    local r, g = math.sin(time) * 0.5 + 0.5, math.cos(time) * 0.5 + 0.5
    gp.set_color(r, g, 0.3, 1.0)
    gp.rotate_at(time, 0.0, 0.0)
    gp.draw_filled_rect(-0.5, -0.5, 1.0, 1.0)

    -- Begin a render pass.
    local pass_action = gfx.new_pass_action()
    gfx.begin_default_pass(pass_action, width, height)
    -- Dispatch all draw commands to Sokol GFX.
    gp.flush()
    -- Finish a draw command queue, clearing it.
    gp.end_()
    -- End render pass.
    gfx.end_pass()
    -- Commit Sokol render.
    gfx.commit()
end

-- Called when the application is initializing.
local function init()
    -- Initialize Sokol GFX.
    gfx.setup_from_app()

    if not gfx.isvalid() then
        error('Failed to create Sokol GFX context!')
    end

    -- Initialize Sokol GP, adjust the size of command buffers for your own use.
    gp.setup()

    if not gp.is_valid() then
        error('Failed to create Sokol GP context: ' .. gp.get_error_message(gp.get_last_error()))
    end
end


-- Called when the application is shutting down.
local function cleanup()
    -- Cleanup Sokol GP and Sokol GFX resources.
    gp.shutdown()
    gfx.shutdown()
end

return {
    init_cb = init,
    frame_cb = frame,
    cleanup_cb = cleanup,
    window_title = 'Triangle (Sokol GP)',
    sample_count = 4 -- Enable anti aliasing.
}
