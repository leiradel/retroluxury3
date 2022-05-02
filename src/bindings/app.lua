local fetch = require 'sokol.fetch'
local done = false
local main = {}

fetch.setup {}

fetch.send {
    path = "main.lua",
    chunk_size = 4096,

    callback = function(response)
        if response.fetched then
            main[#main + 1] = response.buffer
        end

        if response.finished then
            if response.failed or response.cancelled then
                error('could not read main.lua: ' .. fetch.error[response.error_code])
            else
                done = true
            end
        end
    end
}

while not done do
    fetch.dowork()
end

fetch.shutdown()

local chunk, err = load(table.concat(main, ''), 'main.lua', 't')

if not chunk then
    error(err)
end

return chunk
