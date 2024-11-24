component = {}

--create and initialize component flags--
component.flags = {}
component.flags.a = 0
component.flags.b = 1

--create registers--
component.registers = {}
component.registers.x = 0
component.registers.y = 0

component.memory = mem_alloc( 100 )
component.rom = mem_alloc( 100 )
set_bin_target( component.rom )

--(for CPUs) create an opcode--
--note that we pass an argument self; the component that owns this function--
function add( self )
    self.registers.x = self.registers.x + self.registers.y
end

component.op_code_table = {}
component.op_code_table[0] = add

--register component--
register_component( "component", component )
