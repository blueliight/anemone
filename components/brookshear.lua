-- an implementation of the brookshear architecture--

byte_mask_left = 0xF0
Byte_mask_right = 0x0F

system = {}
system.registers = {}

system.memory = mem_alloc( 0xF )

for n=0x0,0xF,0x1 do
    local k = 0
    system.registers[n] = k
end

-- load R, [XY] ;contents
function load()

end

-- load R, XY ;value
function load2()
    
end

-- store R,[XY]
function store()
    
end

-- move S,R
function move()

end

-- addI R,S,T ;2's complement
-- addF R,S,T ;floating point
-- or R,S,T
-- and R,S,T
-- xor R,S,T
-- ROR R,X
-- jmpEQ R=R0,XY
-- halt
-- load R,[S]
-- jumpLE R <= R0,X

-- brookshear instruction codes are 4 bits long, so we need a custom switch
-- to decipher incoming calls

-- we might have to provide byte as a lightuserdata uchar *, then provide functions for
-- handling this data type

-- we could also try overwriting the standard lua functions like tostring()
function brookshear_parse( byte )
    print( "byte " .. to_hex_string(byte) )
    local instruction = byte & byte_mask_left
    local arg1 = byte & Byte_mask_right

    print( "instruction " .. to_hex_string( instruction ) )
    print( "arg1 = " .. to_hex_string( arg1 ) )
end

register_custom_instruction_parser( brookshear_parse )
register_component( "testsystem", system )