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
        elseif response.failed then
            error('' .. response.error_code)
        end

        done = response.finished
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
