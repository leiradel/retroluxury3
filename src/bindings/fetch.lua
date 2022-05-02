return function(fetch)
    fetch.load = function(path, chunk_size, callback)
        local data = {}

        fetch.send {
            path = path,
            chunk_size = chunk_size or 4096,
        
            callback = function(response)
                if response.fetched then
                    data[#data + 1] = response.buffer
                end

                if response.finished then
                    if response.failed or response.cancelled then
                        callback(nil, response.error_code)
                    else
                        callback(table.concat(data, ''))
                    end
                end
            end
        }
    end
end
