#!/usr/bin/python
import BeautifulSoup as bs
import re
opcodes = {}
opcode_type = {}
operand1 = {}
operand2 = {}
timing = {}

def parse_table(table):

    count = 0
    for row in table.findAll('tr')[1:]:
        for col in row.findAll('td')[1:]:
            # Format is TYPE OPERAND1, OPERAND2
            if col.text == '&nbsp;':
                opcode_type[count] = 'UNDEFINED'
                operand1[count] = 'NONE'
                operand2[count] = 'NONE'
                timing[count] = 0
            else:
                # OPCODE contents
                opcode_info = col.contents[0].split(' ')
                opcode_type[count] = str(opcode_info[0]) # get the opcode type
                if len(opcode_info) > 1:
                    operands = opcode_info[1].split(',')
                    operand1[count] = str(operands[0])
                    if len(operands) > 1:
                        operand2[count] = str(operands[1])
                    else:
                        operand2[count] = 'NONE'
                else:
                    operand1[count] = 'NONE'
                    operand2[count] = 'NONE'

                # TIMING contents
                timing[count] = re.findall('\d+', col.contents[2])[1]
                #print (timing[count])

            count += 1
    return opcode_type

def create_operand_table(table, table_name):
    s = '#define ' + table_name + ' '
    for operand in table:
        if (table[operand] == 'A'):
            s += 'reg.pA'
        elif (table[operand] == 'B'):
            s += 'reg.pB'
        elif (table[operand] in ['C', '(C)']):
            s += 'reg.pC'
        elif (table[operand] == 'D'):
            s += 'reg.pD'
        elif (table[operand] == 'E'):
            s += 'reg.pE'
        elif (table[operand] == 'F'):
            s += 'reg.pF'
        elif (table[operand] == 'H'):
            s += 'reg.pH'
        elif (table[operand] == 'L'):
            s += 'reg.pL'
        elif ('AF' in table[operand]):
            s += '(unsigned char *) reg.pAF'
        elif ('BC' in table[operand]):
            s += '(unsigned char *) reg.pBC'
        elif ('DE' in table[operand]):
            s += '(unsigned char *) reg.pDE'
        elif ('HL' in table[operand]):
            s += '(unsigned char *) reg.pHL'
        elif ('SP' in table[operand]):
            s += '(unsigned char *) reg.pSP'
        elif ( table[operand] in ['0', '1', '2', '3', '4', '5', '6', '7']):
            s = s + '&const_int_table[' + table[operand] + ']'
        elif ( table[operand] in ['00H', '08H', '10H', '18H', '20H', '28H', '30H', '38H']):
            if (table[operand] == '00H'):
                n = 0
            elif (table[operand] == '08H'):
                n = 8
            elif (table[operand] == '10H'):
                n = 9
            elif (table[operand] == '18H'):
                n = 10
            elif (table[operand] == '20H'):
                n = 11
            elif (table[operand] == '28H'):
                n = 12
            elif (table[operand] == '30H'):
                n = 13
            elif (table[operand] == '38H'):
                n = 14
            s += '&const_int_table[' + str(n) + ']'
        elif ('JP_' in table[operand]):
            s = s + '&const_int_table[' + table[operand] + ']'
        else:
            s += 'NULL'
        if (operand != (len(table) - 1)):
            s += ', '
        else:
            s += '\n\n'
    return s

def create_opcode_type_table(table, table_name):
    s = '#define ' + table_name + ' '
    for operand in table:
        if ((table[operand] in ['ADD', 'SUB', 'LD', 'INC', 'DEC']) and (operand1[operand] in ['BC', 'DE', 'HL', 'SP', 'AL', '(a16)'])):
            if (operand1[operand] == '(a16)' and operand2[operand] == 'A'):
                s += table[operand] 
            else:
                s += table[operand] + '16'
        else:
            s += table[operand] 
        if (operand != (len(table) - 1)):
            s += ', '
        else:
            s += '\n\n'
    return s

def create_operand_type_table(table, table_name):
    s = '#define ' + table_name + ' '
    for operand in table:
        # INC and DEC instructions will be treated as special add/subtract
        if (table[operand] in['0', '1', '2', '3', '4', '5', '6', '7']):
            if (operand1[operand] in ['HL', 'BC', 'DE', 'SP', 'AF']):
                s += 'ABS16_OP'
            else:
                s += 'REG8_OP'
        elif ( table[operand] in ['00H', '08H', '10H', '18H', '20H', '28H', '30H', '38H']):
            s += 'REG8_OP'
        elif (table[operand] in ['JP_ALWAYS', 'JP_Z', 'JP_NZ', 'JP_C', 'JP_NC']):
            s += 'REG8_OP'
        else:
            # regular instructions
            if (table[operand] == '(a16)'):
                if (operand2[operand] == 'SP'):
                    s += 'IMM16_MEM16_OP'
                else:
                    s += 'IMM16_MEM_OP'
            elif (table[operand] == '(C)'):
                s+= 'LD_C_FROM_MEM'
            elif (table[operand] == '(a8)'):
                s += 'IMM8_MEM_OP'
            elif (re.search('(\(.*\))', table[operand])):
                s += 'MEM_OP'
            elif (table[operand] in ['d8', 'r8']):
                s += 'IMM8_OP'
            elif (table[operand] in ['a16', 'd16']):
                s += 'IMM16_OP'
            elif (table[operand] in ['A', 'B', 'C', 'D', 'E', 'F', 'H', 'L']): 
                s += 'REG8_OP'
            elif (table[operand] in ['AF', 'BC', 'DE', 'HL', 'SP']):
                s += 'REG16_OP'
            elif (table[operand] == "SP+r8"):
                s+= 'SP_PLUS_R8_OP' # special case for one instruction
            else:
                s += '0'
        if (operand != (len(table) - 1)):
            s += ', '
        else:
            s += '\n\n'
    return s


# The value is always stored into the first operand, but we still have to know what type it is
def create_result_type(table_name):
    s = '#define ' + table_name + ' '
    for item in operand1:
        if (opcode_type[item] in ['JP', 'BIT', 'CP']):
           s += 'NO_RESULT'     
        elif (operand1[item] in ['A', 'B', 'C', 'D', 'E', 'F', 'H', 'L']):
            s += 'REG8_RESULT'
        elif (operand1[item] in ['AF', 'BC', 'DE', 'HL', 'SP']):
            s += 'REG16_RESULT'
        elif ((operand1[item] in ['(a16)', '(SP)']) or (opcode_type[item] == 'CALL')):
            if (operand1[item] == '(a16)' and operand2[item] == 'A'):
                s += 'MEM8_RESULT' 
            else:
                s += 'MEM16_RESULT'
        elif ('(HL+)' in [operand1[item], operand2[item]]):
            s += 'MEM8_HLPLUS_RESULT'
        elif ('(HL-)' in [operand1[item], operand2[item]]):
            s += 'MEM8_HLMINUS_RESULT'        
        elif (re.search('(\(.*\))', operand1[item])):
            s += 'MEM8_RESULT'
        else:
            s += 'NO_RESULT'
        if (item != (len(operand1) - 1)):
            s += ', '
        else:
            s += '\n\n'
    return s
    
def create_description_table(table_name):
    table_print = '#define ' + table_name + ' '
    for item in opcode_type:
        s = '"' + opcode_type[item]
        if (operand1[item] != 'NONE'):
            s = s + ' ' + operand1[item]
        if (operand2[item] != 'NONE'):
            s = s + ', ' + operand2[item]
        s += '"'
        if (item != (len(operand1) - 1)):
            s += ', '
        else:
            s += '\n\n'
        table_print += s
    return table_print

def create_timing_table(table_name):
    table_print = '#define ' + table_name + ' '
    for item in timing:
        s = str(timing[item])
        if (item != (len(operand1) - 1)):
            s += ', '
        else:
            s += '\n\n'
        table_print += s

    return table_print

# this table converts inc, dec instructions and changes make operand list consistent
def modify_opcode_table():       
    for item in opcode_type:
        if (opcode_type[item] == 'DAA'):
            operand1[item] = 'A'
        if (opcode_type[item] == 'INC'):
            #opcode_type[item] = 'ADD'
            operand2[item] = '1'
        elif (opcode_type[item] == 'DEC'):
            #opcode_type[item] = 'SUB'
            operand2[item] = '1'
        elif (opcode_type[item] == 'LDH'):
            opcode_type[item] = 'LD'
        elif (opcode_type[item] == 'PUSH'):    
            operand2[item] = operand1[item]
            operand1[item] = '(SP)'
        elif (opcode_type[item] == 'POP'):    
            operand2[item] = '(SP)'
        elif (opcode_type[item] in ['AND', 'OR', 'XOR', 'CP', 'SUB']):
            operand2[item] = operand1[item]
            operand1[item] = 'A'
        elif (opcode_type[item] in ['RLCA', 'RLA', 'RRCA', 'RRA', 'CPL']):
            operand1[item] = 'A'
            if (opcode_type[item] == 'RLCA'):
                opcode_type[item] = 'RLC'
            elif (opcode_type[item] == 'RLA'):
                opcode_type[item] = 'RL'
            elif (opcode_type[item] == 'RRCA'):
                opcode_type[item] = 'RRC'
            elif (opcode_type[item] == 'RRA'):
                opcode_type[item] = 'RR'
        elif (opcode_type[item] in ['JP', 'JR', 'CALL']):
            if (operand1[item] in ['(a16)', 'a16', '(HL)', 'r8']):
                operand2[item] = operand1[item]
                operand1[item] = 'JP_ALWAYS'
                if (operand2[item] == '(HL)'):
                    operand2[item] = 'HL'
            elif (operand1[item] in ['Z', 'C', 'NC', 'NZ']):
                operand1[item] = 'JP_' + operand1[item]
        elif (opcode_type[item] == 'RET'):
            if (operand1[item] == 'NONE'):
                operand1[item] = 'JP_ALWAYS'
            else:
               operand1[item] = 'JP_' + operand1[item] 
#        elif (opcode_type[item] == 'RST'):
#            print(operand1[item])
#            operand1[item] =  re.findall('\d+', operand1[item])[0]
#            print(operand1[item])
            
html = open('gameboy_opcodes.html', 'r').read()

soup = bs.BeautifulSoup(html)

table_unprefixed, table_cbprefixed = soup.findAll('table')[:2]

parse_table(table_unprefixed)

s = '#include "defines.h" \n\n'

modify_opcode_table()

s += create_description_table('TABLE_OPCODE_DESCRIPTION')
s +=  create_operand_type_table(operand1, 'TABLE_OPERAND1_TYPE')
s += create_operand_type_table(operand2, 'TABLE_OPERAND2_TYPE')

s += create_operand_table(operand1, 'TABLE_OPERAND1')
s += create_operand_table(operand2, 'TABLE_OPERAND2')

s += create_opcode_type_table(opcode_type, 'TABLE_OP_TYPE')

s += create_result_type('TABLE_RESULT_TYPE')

s += create_timing_table('TABLE_CYCLE_COUNT')

f = open('table_defines.h', 'w')
f.write(s)


# *************************************
# PREFIX CB RESULTS FROM NOW ON
# *************************************
def modify_cb_opcode_table():       
    for item in opcode_type:
        if (opcode_type[item] in ['BIT', 'RES', 'SET']):
            operand1[item], operand2[item] = operand2[item], operand1[item]

parse_table(table_cbprefixed)

modify_cb_opcode_table()

cb_s = create_description_table('TABLE_OPCODE_DESCRIPTION_CB')
cb_s +=  create_operand_type_table(operand1, 'TABLE_OPERAND1_TYPE_CB')
cb_s += create_operand_type_table(operand2, 'TABLE_OPERAND2_TYPE_CB')

cb_s += create_operand_table(operand1, 'TABLE_OPERAND1_CB')
cb_s += create_operand_table(operand2, 'TABLE_OPERAND2_CB')

cb_s += create_opcode_type_table(opcode_type, 'TABLE_OP_TYPE_CB')

cb_s += create_result_type('TABLE_RESULT_TYPE_CB')

cb_s += create_timing_table('TABLE_CYCLE_COUNT_CB')

f.write(cb_s)
