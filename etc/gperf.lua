local def = [[
    uint64_t frame_count;               // current frame counter, always valid, useful for checking if two events were issued in the same frame
    enum type;               // the event type, always valid
    enum key_code;              // the virtual key code, only valid in KEY_UP, KEY_DOWN
    uint32_t char_code;                 // the UTF-32 character code, only valid in CHAR events
    bool key_repeat;                    // true if this is a key-repeat event, valid in KEY_UP, KEY_DOWN and CHAR
    uint32_t modifiers;                 // current modifier keys, valid in all key-, char- and mouse-events
    enum mouse_button;      // mouse button that was pressed or released, valid in MOUSE_DOWN, MOUSE_UP
    float mouse_x;                      // current horizontal mouse position in pixels, always valid except during mouse lock
    float mouse_y;                      // current vertical mouse position in pixels, always valid except during mouse lock
    float mouse_dx;                     // relative horizontal mouse movement since last frame, always valid
    float mouse_dy;                     // relative vertical mouse movement since last frame, always valid
    float scroll_x;                     // horizontal mouse wheel scroll distance, valid in MOUSE_SCROLL events
    float scroll_y;                     // vertical mouse wheel scroll distance, valid in MOUSE_SCROLL events
    int num_touches;                    // number of valid items in the touches[] array
    struct touches[SAPP_MAX_TOUCHPOINTS];  // current touch points, valid in TOUCHES_BEGIN, TOUCHES_MOVED, TOUCHES_ENDED
    int window_width;                   // current window- and framebuffer sizes in pixels, always valid
    int window_height;
    int framebuffer_width;              // = window_width * dpi_scale
    int framebuffer_height;             // = window_height * dpi_scale
]]

local function djb2(str)
    local hash = 5381

    for i = 1, #str do
        local k = str:byte(i)
        hash = (hash * 33 + k) & 0xffffffff
    end

    return hash
end

local function parse(struct, fields)
    local types = {
        ['bool'] = 'LUTIL_BOOL',
        ['int'] = 'LUTIL_INT',
        ['enum'] = 'LUTIL_ENUM',
        ['float'] = 'LUTIL_FLOAT',
        ['uint32_t'] = 'LUTIL_U32',
        ['uint64_t'] = 'LUTIL_U64',
        ['uintptr_t'] = 'LUTIL_UPTR'
    }

    local data, count = {}, 0

    for line in fields:gmatch('(.-)\n') do
        local decl, id, array = line:match('%s*([%a_][%w_]*%s*%*?)%s*([%a_][%w_]*)%[([%a_][%w_]*)%]')

        if not decl then
            decl, id = line:match('%s*([%a_][%w_]*%s*%*?)%s*([%a_][%w_]*)')
        end

        local type = decl:match('([%a_][%w_]*)')
        local pointer = decl:find('*', 1, true) ~= nil
        
        print(decl, id, array, type, pointer)

        data[id] = {
            id = id,
            type = types[type] or 'LUTIL_STRUCT',
            struct = type,
            pointer = pointer,
            array = array,
            hash = djb2(id),
            index = count
        }

        data[count] = data[id]
        count = count + 1
    end

    return data, count
end

local function emit(struct, fields, total)
    local count, hashes = total, nil

    while true do
        hashes = {}
        local found = true

        for id, info in pairs(fields) do
            if type(id) ~= 'number' then
                local position = info.hash % count

                if hashes[position] then
                    found = false
                    break
                end

                hashes[position] = info
            end
        end

        if found then
            break
        end

        count = count + 1
    end

    io.write(string.format('static lutil_FieldDesc const %s_fields[%d] = {\n', struct, total))

    for index = 0, total - 1 do
        local info = fields[index]
        local type = info.pointer and string.format('%s | LUTIL_POINTER', info.type) or info.type
        local desc = info.type == 'LUTIL_STRUCT' and string.format('&%s_desc', info.struct) or 'NULL'
        local array = info.array and (fields[info.array].index + 1) or 0

        io.write(string.format(
            '    /* %3d */ {DJB2HASH_C(0x%08x), %s, "%s", LUTIL_OFS(%s, %s), %s, %d},\n',
            index + 1,
            info.hash,
            type,
            info.id,
            struct,
            info.id,
            desc,
            array
        ))
    end

    io.write(string.format('};\n\n'))

    io.write(string.format('static uint8_t const %s_field_lookup[%d] = {\n', struct, count))
    io.write(string.format('    '))
    local comma = ''

    for position = 0, count - 1 do
        local info = hashes[position]

        if info then
            io.write(string.format('%s%d', comma, info.index + 1))
        else
            io.write(string.format('%s0', comma))
        end

        comma = ', '
    end

    io.write(string.format('\n};\n\n'))

    io.write(string.format('static lutil_StructDesc %s_desc = {\n', struct))
    io.write(string.format('    "%s",\n', struct))
    io.write(string.format('    sizeof(%s),\n', struct))
    io.write(string.format('    sizeof(%s_fields) / sizeof(%s_fields[0]),\n', struct, struct))
    io.write(string.format('    %s_fields,\n', struct))
    io.write(string.format('    %d,\n', count))
    io.write(string.format('    %s_field_lookup,\n', struct))
    io.write(string.format('    NULL\n'))
    io.write(string.format('\n};\n'))
end

local fields, total = parse('sapp_event', def)
emit('sapp_event', fields, total)
