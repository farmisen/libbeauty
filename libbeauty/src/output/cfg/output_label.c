/*
 *  Copyright (C) 2012  The libbeauty Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * 06-05-2012 Initial work.
 *   Copyright (C) 2012 James Courtier-Dutton James@superbug.co.uk
 */

#include <stdio.h>
#include <rev.h>

extern int reg_params_order[];

const char *condition_table[] = {
	"OVERFLOW_0", /* Signed */
	"NOT_OVERFLOW_1", /* Signed */
	"BELOW_2",	/* Unsigned */
	"NOT_BELOW_3",	/* Unsigned */
	"EQUAL_4",	/* Signed or Unsigned */
	"NOT_EQUAL_5",	/* Signed or Unsigned */
	"BELOW_EQUAL_6",	/* Unsigned */
	"ABOVE_7",	/* Unsigned */
	"SIGNED_8",	/* Signed */
	"NO_SIGNED_9",	/* Signed */
	"PARITY_10",	/* Signed or Unsigned */
	"NOT_PARITY_11",/* Signed or Unsigned */
	"LESS_12",	/* Signed */
	"GREATER_EQUAL_13", /* Signed */
	"LESS_EQUAL_14",    /* Signed */
	"GREATER_15"	/* Signed */
};

int label_to_string(struct label_s *label, char *string, int size) {
	int tmp;
	int offset = 0;

	switch (label->scope) {
	case 3:
		debug_print(DEBUG_OUTPUT, 1, "%"PRIx64";\n", label->value);
		/* FIXME: Handle the case of an immediate value being &data */
		/* but it is very difficult to know if the value is a pointer (&data) */
		/* or an offset (data[x]) */
		/* need to use the relocation table to find out */
		/* no relocation table entry == offset */
		/* relocation table entry == pointer */
		/* this info should be gathered at disassembly point */
		switch (label->type) {
		case 1:
			tmp = snprintf(&string[offset], size - offset, "data%04"PRIx64,
				label->value);
			offset += tmp;
			if (offset >= size) {
				return 1;
			}
			break;
		case 2:
			tmp = snprintf(&string[offset], size - offset, "&data%04"PRIx64,
				label->value);
			offset += tmp;
			if (offset >= size) {
				return 1;
			}
			break;
		case 3:
			tmp = snprintf(&string[offset], size - offset, "0x%"PRIx64,
				label->value);
			offset += tmp;
			if (offset >= size) {
				return 1;
			}
			break;
		default:
			debug_print(DEBUG_OUTPUT, 1, "output_label error\n");
			return 1;
			break;
		}
		break;
	case 2:
		switch (label->type) {
		case 2:
			debug_print(DEBUG_OUTPUT, 1, "param_stack%04"PRIx64"\n",
				label->value);
			tmp = snprintf(&string[offset], size - offset, "param_stack%04"PRIx64,
				label->value);
			offset += tmp;
			if (offset >= size) {
				return 1;
			}
			break;
		case 1:
			debug_print(DEBUG_OUTPUT, 1, "param_reg%04"PRIx64"\n",
				label->value);
			tmp = snprintf(&string[offset], size - offset, "param_reg%04"PRIx64,
				label->value);
			offset += tmp;
			if (offset >= size) {
				return 1;
			}
			break;
		default:
			debug_print(DEBUG_OUTPUT, 1, "output_label error\n");
			return 1;
			break;
		}
		break;
	case 1:
		switch (label->type) {
		case 2:
			debug_print(DEBUG_OUTPUT, 1, "local_stack%04"PRIx64"\n",
				label->value);
			tmp = snprintf(&string[offset], size - offset, "local_stack%04"PRIx64,
				label->value);
			offset += tmp;
			if (offset >= size) {
				return 1;
			}
			break;
		case 1:
			debug_print(DEBUG_OUTPUT, 1, "local_reg%04"PRIx64"\n",
				label->value);
			tmp = snprintf(&string[offset], size - offset, "local_reg%04"PRIx64,
				label->value);
			offset += tmp;
			if (offset >= size) {
				return 1;
			}
			break;
		default:
			debug_print(DEBUG_OUTPUT, 1, "output_label error type=%"PRIx64"\n", label->type);
			return 1;
			break;
		}
		break;
	case 4:
		/* FIXME: introduce indirect_value_id and indirect_value_scope */
		/* in order to resolve somewhere */
		/* It will always be a register, and therefore can re-use the */
		/* value_id to identify it. */
		/* It will always be a local and not a param */
		/* FIXME: local_reg should be handled in case 1.1 above and
		 *        not be a separate label
		 */
		debug_print(DEBUG_OUTPUT, 1, "xxxlocal_reg%04"PRIx64";\n", label->value);
		tmp = snprintf(&string[offset], size - offset, "xxxlocal_reg%04"PRIx64,
			label->value);
		offset += tmp;
		if (offset >= size) {
			return 1;
		}
		break;
	default:
		debug_print(DEBUG_OUTPUT, 1, "unknown label scope: %04"PRIx64";\n", label->scope);
		tmp = snprintf(&string[offset], size - offset, "unknown%04"PRIx64"%04"PRIx64,
			label->scope,
			label->value);
		offset += tmp;
		if (offset >= size) {
			return 1;
		}
		break;
	}
	return 0;
}

int output_label_redirect(int offset, struct label_s *labels, struct label_redirect_s *label_redirect, FILE *fd) {
	int tmp;
	struct label_s *label;
	char buffer[1024];

	tmp = label_redirect[offset].redirect;
	label = &labels[tmp];
	tmp = label_to_string(label, buffer, 1023);
	if (tmp) {
		return tmp;
	}
	tmp = fprintf(fd, "%s", buffer);
	return 0;
}

int output_variable(int store, int indirect, uint64_t index, uint64_t relocated, uint64_t value_scope, uint64_t value_id, uint64_t indirect_offset_value, uint64_t indirect_value_id, FILE *fd) {
	int tmp;
	/* FIXME: May handle by using first switch as switch (indirect) */
	switch (store) {
	case STORE_DIRECT:
		debug_print(DEBUG_OUTPUT, 1, "%"PRIx64";\n", index);
		/* FIXME: Handle the case of an immediate value being &data */
		/* but it is very difficult to know if the value is a pointer (&data) */
		/* or an offset (data[x]) */
		/* need to use the relocation table to find out */
		/* no relocation table entry == offset */
		/* relocation table entry == pointer */
		/* this info should be gathered at disassembly point */
		if (indirect == IND_MEM) {
			tmp = fprintf(fd, "data%04"PRIx64,
				index);
		} else if (relocated) {
			tmp = fprintf(fd, "&data%04"PRIx64,
				index);
		} else {
			tmp = fprintf(fd, "0x%"PRIx64,
				index);
		}
		break;
	case STORE_REG:
		switch (value_scope) {
		case 1:
			/* FIXME: Should this be param or instead param_reg, param_stack */
			if (IND_STACK == indirect) {
				debug_print(DEBUG_OUTPUT, 1, "param_stack%04"PRIx64",%04"PRIx64",%04d\n",
					index, indirect_offset_value, indirect);
				tmp = fprintf(fd, "param_stack%04"PRIx64",%04"PRIx64",%04d",
					index, indirect_offset_value, indirect);
			} else if (0 == indirect) {
				debug_print(DEBUG_OUTPUT, 1, "param_reg%04"PRIx64,
					index);
				tmp = fprintf(fd, "param_reg%04"PRIx64,
					index);
			}
			break;
		case 2:
			/* FIXME: Should this be local or instead local_reg, local_stack */
			if (IND_STACK == indirect) {
				debug_print(DEBUG_OUTPUT, 1, "local_stack%04"PRIx64"\n",
					value_id);
				tmp = fprintf(fd, "local_stack%04"PRIx64,
					value_id);
			} else if (0 == indirect) {
				debug_print(DEBUG_OUTPUT, 1, "local_reg%04"PRIx64"\n",
					value_id);
				tmp = fprintf(fd, "local_reg%04"PRIx64,
					value_id);
			}
			break;
		case 3: /* Data */
			/* FIXME: introduce indirect_value_id and indirect_value_scope */
			/* in order to resolve somewhere */
			/* It will always be a register, and therefore can re-use the */
			/* value_id to identify it. */
			/* It will always be a local and not a param */
			debug_print(DEBUG_OUTPUT, 1, "xxxlocal_mem%04"PRIx64";\n", (indirect_value_id));
			tmp = fprintf(fd, "xxxlocal_mem%04"PRIx64,
				indirect_value_id);
			break;
		default:
			debug_print(DEBUG_OUTPUT, 1, "unknown value scope: %04"PRIx64";\n", (value_scope));
			tmp = fprintf(fd, "unknown%04"PRIx64,
				value_scope);
			break;
		}
		break;
	default:
		debug_print(DEBUG_OUTPUT, 1, "Unhandled store1\n");
		break;
	}
	return 0;
}

int if_expression( int condition, struct inst_log_entry_s *inst_log1_flagged,
	struct label_redirect_s *label_redirect, struct label_s *labels, FILE *fd)
{
	int opcode;
	int err = 0;
	int tmp;
	//int store;
	//int indirect;
	//uint64_t index;
	//uint64_t relocated;
	//uint64_t value_scope;
	uint64_t value_id;
	//uint64_t indirect_offset_value;
	//uint64_t indirect_value_id;
	struct label_s *label;
	const char *condition_string;
	char buffer[1024];

	opcode = inst_log1_flagged->instruction.opcode;
	debug_print(DEBUG_OUTPUT, 1, "\t if opcode=0x%x, \n",inst_log1_flagged->instruction.opcode);

	switch (opcode) {
	case CMP:
	case ICMP:
		switch (condition) {
		case LESS_EQUAL:
		case BELOW_EQUAL:   /* Unsigned */
			condition_string = " <= ";
			break;
		case GREATER_EQUAL: /* Signed */
		case NOT_BELOW:   /* Unsigned */
			condition_string = " >= ";
			break;
		case GREATER:
		case ABOVE:
			condition_string = " > ";
			break;
		case BELOW:
		case LESS:
			condition_string = " < ";
			break;
		case NOT_EQUAL:
			condition_string = " != ";
			break;
		case EQUAL:
			condition_string = " == ";
			break;
		default:
			debug_print(DEBUG_OUTPUT, 1, "if_expression: non-yet-handled: 0x%x\n", condition);
			err = 1;
			break;
		}
		if (err) break;
		tmp = fprintf(fd, "(");

		switch (inst_log1_flagged->instruction.srcB.indirect) {
		case IND_MEM:
		case IND_IO:
			tmp = fprintf(fd, "*");
			/* fall through */
		case IND_STACK:
			value_id = inst_log1_flagged->value2.indirect_value_id;
			break;
		case IND_DIRECT:
			value_id = inst_log1_flagged->value2.value_id;
			break;
		}
		if (STORE_DIRECT == inst_log1_flagged->instruction.srcB.store) {
			tmp = fprintf(fd, "0x%"PRIx64, inst_log1_flagged->instruction.srcB.index);
		} else {
			char buffer[1024];
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
		}
		tmp = fprintf(fd, "%s", condition_string);

		switch (inst_log1_flagged->instruction.srcA.indirect) {
		case IND_MEM:
		case IND_IO:
			tmp = fprintf(fd, "*");
			/* fall through */
		case IND_STACK:
			value_id = inst_log1_flagged->value1.indirect_value_id;
			break;
		case IND_DIRECT:
			value_id = inst_log1_flagged->value1.value_id;
			break;
		}

		if (STORE_DIRECT == inst_log1_flagged->instruction.srcB.store) {
			tmp = fprintf(fd, "0x%"PRIx64, inst_log1_flagged->instruction.srcB.index);
		} else {
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
		}

		tmp = fprintf(fd, ") ");
		break;
	case SUB:
	case ADD:
		switch (condition) {
		case EQUAL:
			condition_string = " == 0";
			break;
		case NOT_EQUAL:
			condition_string = " != 0";
			break;
		default:
			debug_print(DEBUG_OUTPUT, 1, "if_expression: non-yet-handled: 0x%x\n", condition);
			err = 1;
			break;
		}

		if ((!err) && (IND_DIRECT == inst_log1_flagged->instruction.srcA.indirect) &&
			(IND_DIRECT == inst_log1_flagged->instruction.dstA.indirect) &&
			(0 == inst_log1_flagged->value3.offset_value)) {
			tmp = fprintf(fd, "((");
			if (1 == inst_log1_flagged->instruction.dstA.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1_flagged->value2.indirect_value_id;
			} else {
				value_id = inst_log1_flagged->value2.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			tmp = fprintf(fd, "%s) ", condition_string);
		}
		break;

	case TEST:
		switch (condition) {
		case EQUAL:
			condition_string = " == 0";
			break;
		case NOT_EQUAL:
			condition_string = " != 0";
			break;
		case LESS_EQUAL:
			condition_string = " <= 0";
			break;
		default:
			debug_print(DEBUG_OUTPUT, 1, "if_expression: non-yet-handled: 0x%x\n", condition);
			err = 1;
			break;
		}

		if ((!err) && (IND_DIRECT == inst_log1_flagged->instruction.srcA.indirect) &&
			(IND_DIRECT == inst_log1_flagged->instruction.dstA.indirect) &&
			(0 == inst_log1_flagged->value3.offset_value)) {
			tmp = fprintf(fd, "((");
			if (1 == inst_log1_flagged->instruction.dstA.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1_flagged->value2.indirect_value_id;
			} else {
				value_id = inst_log1_flagged->value2.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			tmp = fprintf(fd, " AND ");
			if (1 == inst_log1_flagged->instruction.srcA.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1_flagged->value1.indirect_value_id;
			} else {
				value_id = inst_log1_flagged->value1.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			tmp = fprintf(fd, ")%s) ", condition_string);
		}
		break;

	case rAND:
		switch (condition) {
		case EQUAL:
			condition_string = " == 0";
			break;
		case NOT_EQUAL:
			condition_string = " != 0";
			break;
		default:
			debug_print(DEBUG_OUTPUT, 1, "if_expression: non-yet-handled: 0x%x\n", condition);
			err = 1;
			break;
		}

		if ((!err) && (IND_DIRECT == inst_log1_flagged->instruction.srcA.indirect) &&
			(IND_DIRECT == inst_log1_flagged->instruction.dstA.indirect) &&
			(0 == inst_log1_flagged->value3.offset_value)) {
			tmp = fprintf(fd, "((");
			if (1 == inst_log1_flagged->instruction.dstA.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1_flagged->value2.indirect_value_id;
			} else {
				value_id = inst_log1_flagged->value2.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			tmp = fprintf(fd, " AND ");
			if (1 == inst_log1_flagged->instruction.srcA.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1_flagged->value1.indirect_value_id;
			} else {
				value_id = inst_log1_flagged->value1.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			tmp = fprintf(fd, ")%s) ", condition_string);
		}
		break;

	default:
		debug_print(DEBUG_OUTPUT, 1, "if_expression: Previous flags instruction not handled: opcode = 0x%x, cond = 0x%x\n", opcode, condition);
		err = 1;
		break;
	}
	return err;
}

uint32_t output_function_name(FILE *fd,
		struct external_entry_point_s *external_entry_point)
{
	int tmp;

	debug_print(DEBUG_OUTPUT, 1, "int %s()\n{\n", external_entry_point->name);
	debug_print(DEBUG_OUTPUT, 1, "value = %"PRIx64"\n", external_entry_point->value);
	tmp = fprintf(fd, "int %s(", external_entry_point->name);
	return 0;
}

int output_3_labels(struct self_s *self, FILE *fd, struct inst_log_entry_s *inst_log1, int inst_number,
        struct label_redirect_s *label_redirect, struct label_s *labels, const char *symbol, const char *cr, char *buffer) {
	struct instruction_s *instruction = &(inst_log1->instruction);
	int tmp;
	uint64_t value_id;
	struct label_s *label;
	if (instruction->srcA.indirect) {
		debug_print(DEBUG_OUTPUT, 1, "Illegal indirect\n");
		exit(1);
	}
	if (instruction->srcB.indirect) {
		debug_print(DEBUG_OUTPUT, 1, "Illegal indirect\n");
		exit(1);
	}
	if (instruction->dstA.indirect) {
		debug_print(DEBUG_OUTPUT, 1, "Illegal indirect\n");
		exit(1);
	}
	if (print_inst(self, instruction, inst_number, labels))
		return 1;
	debug_print(DEBUG_OUTPUT, 1, "\t");
	tmp = fprintf(fd, "\t");
	value_id = inst_log1->value3.value_id;
	tmp = label_redirect[value_id].redirect;
	label = &labels[tmp];
	tmp = label_to_string(label, buffer, 1023);
	tmp = fprintf(fd, "%s", buffer);
	//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value3.value_id);
	tmp = fprintf(fd, " = ");
	value_id = inst_log1->value1.value_id;
	tmp = label_redirect[value_id].redirect;
	label = &labels[tmp];
	tmp = label_to_string(label, buffer, 1023);
	tmp = fprintf(fd, "%s", buffer);
	tmp = fprintf(fd, " %s ", symbol);
	debug_print(DEBUG_OUTPUT, 1, "\nstore=%d\n", instruction->srcA.store);
	value_id = inst_log1->value2.value_id;
	tmp = label_redirect[value_id].redirect;
	label = &labels[tmp];
	tmp = label_to_string(label, buffer, 1023);
	tmp = fprintf(fd, "%s", buffer);
	//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value1.value_id);
	tmp = fprintf(fd, ";%s",cr);
	return 0;
}

int output_inst_in_c(struct self_s *self, struct process_state_s *process_state,
			 FILE *fd, int inst_number, struct label_redirect_s *label_redirect, struct label_s *labels, const char *cr)
{
	int tmp, l, n2;
	int tmp_state;
	int err;
	int found;
	uint64_t value_id;
	struct instruction_s *instruction;
	struct inst_log_entry_s *inst_log1 = NULL;
//	struct inst_log_entry_s *inst_log1_prev;
	struct inst_log_entry_s *inst_log1_flags = NULL;
	struct inst_log_entry_s *inst_log_entry = self->inst_log_entry;
	struct external_entry_point_s *external_entry_points = self->external_entry_points;
	struct label_s *label;
	char buffer[1024];
	struct string_s string1;
	string1.len = 0;
	string1.max = 1023;
	string1.string[0] = 0;

	inst_log1 =  &inst_log_entry[inst_number];
	if (!inst_log1) {
		debug_print(DEBUG_OUTPUT, 1, "output_function_body:Invalid inst_log1[0x%x]\n", inst_number);
		return 1;
	}
//	inst_log1_prev =  &inst_log_entry[inst_log1->prev[0]];
//	if (!inst_log1_prev) {
//		debug_print(DEBUG_OUTPUT, 1, "output_function_body:Invalid inst_log1_prev[0x%x]\n", inst_number);
//		return 1;
//	}
	instruction =  &inst_log1->instruction;
	//instruction_prev =  &inst_log1_prev->instruction;

	write_inst(self, &string1, instruction, inst_number, labels);
	tmp = fprintf(fd, "%s", string1.string);
	tmp = fprintf(fd, "%s", cr);
#if 0
	tmp = fprintf(fd, "// ");
	if (inst_log1->prev_size > 0) {
		tmp = fprintf(fd, "prev_size=0x%x: ",
			inst_log1->prev_size);
		for (l = 0; l < inst_log1->prev_size; l++) {
			tmp = fprintf(fd, "prev=0x%x, ",
			inst_log1->prev[l]);
		}
	}
	if (inst_log1->next_size > 0) {
		tmp = fprintf(fd, "next_size=0x%x: ",
			inst_log1->next_size);
		for (l = 0; l < inst_log1->next_size; l++) {
			tmp = fprintf(fd, "next=0x%x, ",
			inst_log1->next[l]);
		}
	}
	tmp = fprintf(fd, "%s", cr);
#endif
	/* Output labels when this is a join point */
	/* or when the previous instruction was some sort of jump */
	if ((inst_log1->prev_size) > 1) {
		debug_print(DEBUG_OUTPUT, 1, "label%04"PRIx32":\n", inst_number);
		tmp = fprintf(fd, "label%04"PRIx32":%s", inst_number, cr);
	} else if (1 == inst_log1->prev_size){
		if ((inst_log1->prev[0] != (inst_number - 1)) &&
			(inst_log1->prev[0] != 0)) {
			debug_print(DEBUG_OUTPUT, 1, "label%04"PRIx32":\n", inst_number);
			tmp = fprintf(fd, "label%04"PRIx32":%s", inst_number, cr);
		}
	}
	debug_print(DEBUG_OUTPUT, 1, "\n");
	/* Test to see if we have an instruction to output */
	debug_print(DEBUG_OUTPUT, 1, "Inst 0x%04x: %d: value_type = %d, %d, %d\n", inst_number,
		instruction->opcode,
		inst_log1->value1.value_type,
		inst_log1->value2.value_type,
		inst_log1->value3.value_type);
	/* FIXME: JCD: This fails for some call instructions */
	if ((0 == inst_log1->value3.value_type) ||
		(1 == inst_log1->value3.value_type) ||
		(2 == inst_log1->value3.value_type) ||
		(3 == inst_log1->value3.value_type) ||
		(4 == inst_log1->value3.value_type) ||
		(6 == inst_log1->value3.value_type) ||
		(5 == inst_log1->value3.value_type)) {
		//tmp = fprintf(fd, "//");
		switch (instruction->opcode) {
		case LOAD:
		case STORE:
			if (inst_log1->value1.value_type == 6) {
				debug_print(DEBUG_OUTPUT, 1, "ERROR1 %d\n", instruction->opcode);
				//break;
			}
			if (inst_log1->value1.value_type == 5) {
				debug_print(DEBUG_OUTPUT, 1, "ERROR2\n");
				//break;
			}
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			debug_print(DEBUG_OUTPUT, 1, "\t");
			tmp = fprintf(fd, "\t");
			/* FIXME: Check limits */
			switch (instruction->dstA.indirect) {
			case IND_MEM:
				tmp = fprintf(fd, "*");
				value_id = inst_log1->value3.value_id;
				break;
			case IND_STACK:
				/* FIXME: only use the indirect_value_id if it is an in-variant
				 *	within the scope of the function.
				 *	Assume in-variant for now.
				 */
				value_id = inst_log1->value3.indirect_value_id;
				break;
			case IND_IO:
				tmp = fprintf(fd, "*");
				value_id = inst_log1->value3.value_id;
				break;
			case IND_DIRECT:
				value_id = inst_log1->value3.value_id;
				break;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value3.value_id);
			tmp = fprintf(fd, " = ");
			debug_print(DEBUG_OUTPUT, 1, "\nstore=%d\n", instruction->srcA.store);
			switch (instruction->srcA.indirect) {
			case IND_MEM:
				tmp = fprintf(fd, "*");
				value_id = inst_log1->value1.value_id;
				//debug_print(DEBUG_OUTPUT, 1, "IND_MEM: inst:0x%x, value_id = 0x%lx\n", inst_number, value_id);
				break;
			case IND_STACK:
				//tmp = fprintf(fd, "stack_");
				value_id = inst_log1->value1.indirect_value_id;
				break;
			case IND_IO:
				value_id = inst_log1->value1.indirect_value_id;
				break;
			case IND_DIRECT:
				value_id = inst_log1->value1.value_id;
				break;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value1.value_id);
			tmp = fprintf(fd, ";%s",cr);

			break;
		case MOV:
		case SEX:
			if (inst_log1->value1.value_type == 6) {
				debug_print(DEBUG_OUTPUT, 1, "ERROR1 %d\n", instruction->opcode);
				//break;
			}
			if (inst_log1->value1.value_type == 5) {
				debug_print(DEBUG_OUTPUT, 1, "ERROR2\n");
				//break;
			}
			if (instruction->srcA.indirect) {
				debug_print(DEBUG_OUTPUT, 1, "Illegal indirect\n");
				exit(1);
			}
			if (instruction->dstA.indirect) {
				debug_print(DEBUG_OUTPUT, 1, "Illegal indirect\n");
				exit(1);
			}
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			debug_print(DEBUG_OUTPUT, 1, "\t");
			tmp = fprintf(fd, "\t");
			value_id = inst_log1->value3.value_id;
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value3.value_id);
			tmp = fprintf(fd, " = ");
			debug_print(DEBUG_OUTPUT, 1, "\nstore=%d\n", instruction->srcA.store);
			value_id = inst_log1->value1.value_id;
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value1.value_id);
			tmp = fprintf(fd, ";%s",cr);

			break;
		case NEG:
			if (inst_log1->value1.value_type == 6) {
				debug_print(DEBUG_OUTPUT, 1, "ERROR1\n");
				//break;
			}
			if (inst_log1->value1.value_type == 5) {
				debug_print(DEBUG_OUTPUT, 1, "ERROR2\n");
				//break;
			}
			if (instruction->srcA.indirect) {
				debug_print(DEBUG_OUTPUT, 1, "Illegal indirect\n");
				exit(1);
			}
			if (instruction->dstA.indirect) {
				debug_print(DEBUG_OUTPUT, 1, "Illegal indirect\n");
				exit(1);
			}
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			debug_print(DEBUG_OUTPUT, 1, "\t");
			tmp = fprintf(fd, "\t");
			value_id = inst_log1->value3.value_id;
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value3.value_id);
			tmp = fprintf(fd, " = 0 -");
			debug_print(DEBUG_OUTPUT, 1, "\nstore=%d\n", instruction->srcA.store);
			value_id = inst_log1->value1.value_id;
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value1.value_id);
			tmp = fprintf(fd, ";%s",cr);

			break;

		case ADD:
			output_3_labels(self, fd, inst_log1, inst_number, label_redirect, labels, "+", cr, buffer);
			break;
		case GEP1:
			if (instruction->srcA.indirect) {
				debug_print(DEBUG_OUTPUT, 1, "Illegal indirect\n");
				exit(1);
			}
			if (instruction->srcB.indirect) {
				debug_print(DEBUG_OUTPUT, 1, "Illegal indirect\n");
				exit(1);
			}
			if (instruction->dstA.indirect) {
				debug_print(DEBUG_OUTPUT, 1, "Illegal indirect\n");
				exit(1);
			}
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			debug_print(DEBUG_OUTPUT, 1, "\t");
			tmp = fprintf(fd, "\t");
			value_id = inst_log1->value3.value_id;
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value3.value_id);
			tmp = fprintf(fd, " = ");
			value_id = inst_log1->value1.value_id;
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			debug_print(DEBUG_OUTPUT, 1, "\nstore=%d\n", instruction->srcA.store);
			value_id = inst_log1->value2.value_id;
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			if (label->value > INT64_MAX) {
				tmp = fprintf(fd, " - 0x%lx", - label->value);
			} else {
				tmp = fprintf(fd, " + 0x%lx", label->value);
			}
			//tmp = label_to_string(label, buffer, 1023);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value1.value_id);
			tmp = fprintf(fd, ";%s",cr);
			break;
		case MUL:
		case IMUL:
			if (instruction->srcA.indirect) {
				debug_print(DEBUG_OUTPUT, 1, "Illegal indirect\n");
				exit(1);
			}
			if (instruction->srcB.indirect) {
				debug_print(DEBUG_OUTPUT, 1, "Illegal indirect\n");
				exit(1);
			}
			if (instruction->dstA.indirect) {
				debug_print(DEBUG_OUTPUT, 1, "Illegal indirect\n");
				exit(1);
			}
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			debug_print(DEBUG_OUTPUT, 1, "\t");
			tmp = fprintf(fd, "\t");
			value_id = inst_log1->value3.value_id;
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value3.value_id);
			tmp = fprintf(fd, " = ");
			debug_print(DEBUG_OUTPUT, 1, "\nstore=%d\n", instruction->srcA.store);
			value_id = inst_log1->value1.value_id;
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			tmp = fprintf(fd, " * ");
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value1.value_id);
			if (1 == instruction->srcB.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1->value2.indirect_value_id;
			} else {
				value_id = inst_log1->value2.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value1.value_id);
			tmp = fprintf(fd, ";%s", cr);
			break;

		case SUB:
		case SBB:
			output_3_labels(self, fd, inst_log1, inst_number, label_redirect, labels, "-", cr, buffer);
			break;
		case rAND:
			output_3_labels(self, fd, inst_log1, inst_number, label_redirect, labels, "&", cr, buffer);
			break;
		case OR:
			output_3_labels(self, fd, inst_log1, inst_number, label_redirect, labels, "|", cr, buffer);
			break;
		case XOR:
			output_3_labels(self, fd, inst_log1, inst_number, label_redirect, labels, "^", cr, buffer);
			break;
		case NOT:
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			debug_print(DEBUG_OUTPUT, 1, "\t");
			tmp = fprintf(fd, "\t");
			if (1 == instruction->dstA.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1->value3.indirect_value_id;
			} else {
				value_id = inst_log1->value3.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value3.value_id);
			tmp = fprintf(fd, " = !");
			debug_print(DEBUG_OUTPUT, 1, "\nstore=%d\n", instruction->srcA.store);
			if (1 == instruction->srcA.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1->value1.indirect_value_id;
			} else {
				value_id = inst_log1->value1.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value1.value_id);
			tmp = fprintf(fd, ";%s",cr);
			break;
		case SHL: //TODO: UNSIGNED
			output_3_labels(self, fd, inst_log1, inst_number, label_redirect, labels, "<<", cr, buffer);
			break;
		case SHR: //TODO: UNSIGNED
			output_3_labels(self, fd, inst_log1, inst_number, label_redirect, labels, ">>", cr, buffer);
			break;
		case SAL: //TODO: SIGNED
			output_3_labels(self, fd, inst_log1, inst_number, label_redirect, labels, "<<", cr, buffer);
			break;
		case SAR: //TODO: SIGNED
			output_3_labels(self, fd, inst_log1, inst_number, label_redirect, labels, ">>", cr, buffer);
			break;
		case JMP:
			debug_print(DEBUG_OUTPUT, 1, "JMP reached XXXX\n");
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			tmp = fprintf(fd, "\t");

//			if (instruction->srcA.relocated) {
//				debug_print(DEBUG_OUTPUT, 1, "JMP goto rel%08"PRIx64";\n", instruction->srcA.index);
//				tmp = fprintf(fd, "JMP goto rel%08"PRIx64";\n",
//					instruction->srcA.index);
//			} else {
				debug_print(DEBUG_OUTPUT, 1, "JMP2 goto label%04"PRIx32";%s",
					inst_log1->next[0], cr);
				tmp = fprintf(fd, "JMP2 goto label%04"PRIx32";%s",
					inst_log1->next[0], cr);
//			}
			break;
		case JMPT:
			debug_print(DEBUG_OUTPUT, 1, "JMPT reached XXXX\n");
			if (inst_log1->value1.value_type == 6) {
				debug_print(DEBUG_OUTPUT, 1, "ERROR1 %d\n", instruction->opcode);
				//break;
			}
			if (inst_log1->value1.value_type == 5) {
				debug_print(DEBUG_OUTPUT, 1, "ERROR2\n");
				//break;
			}
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			debug_print(DEBUG_OUTPUT, 1, "\t");
			tmp = fprintf(fd, "\t");
			/* FIXME: Check limits */
			if (1 == instruction->dstA.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1->value3.indirect_value_id;
			} else {
				value_id = inst_log1->value3.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value3.value_id);
			tmp = fprintf(fd, " = ");
			debug_print(DEBUG_OUTPUT, 1, "\nstore=%d\n", instruction->srcA.store);
			if (1 == instruction->srcA.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1->value1.indirect_value_id;
			} else {
				value_id = inst_log1->value1.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value1.value_id);
			tmp = fprintf(fd, ";%s",cr);
			break;
		case CALL:
			/* FIXME: This does nothing at the moment. */
			if (print_inst(self, instruction, inst_number, labels)) {
				tmp = fprintf(fd, "exiting1\n");
				return 1;
			}
			/* Search for EAX */
			debug_print(DEBUG_OUTPUT, 1, "call index = 0x%"PRIx64"\n", instruction->srcA.index);
			tmp = instruction->srcA.index;
			if ((tmp >= 0) && (tmp < EXTERNAL_ENTRY_POINTS_MAX)) {
				debug_print(DEBUG_OUTPUT, 1, "params size = 0x%x\n",
					external_entry_points[instruction->srcA.index].params_size);
			}
			debug_print(DEBUG_OUTPUT, 1, "\t");
			tmp = fprintf(fd, "\t");
			tmp = label_redirect[inst_log1->value3.value_id].redirect;
			label = &labels[tmp];
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			debug_print(DEBUG_OUTPUT, 1, " = ");
			tmp = fprintf(fd, " = ");
			if (IND_DIRECT == instruction->srcA.indirect) {
				/* A direct call */
				/* FIXME: Get the output right */
				if (1 == instruction->srcA.relocated && inst_log1->extension) {
					struct extension_call_s *call;
					call = inst_log1->extension;
					if (!call) {
						printf("call is NULL\n");
						exit(1);
					}
						
					//tmp = fprintf(fd, "%s(%d:", 
					//	external_entry_points[instruction->srcA.index].name,
					//	external_entry_points[instruction->srcA.index].params_size);
					tmp = fprintf(fd, "%s(", 
						external_entry_points[instruction->srcA.index].name);
					tmp_state = 0;
					for (n2 = 0; n2 < call->params_size; n2++) {
						struct label_s *label;
						tmp = label_redirect[call->params[n2]].redirect;
						label = &labels[tmp];
						//debug_print(DEBUG_OUTPUT, 1, "reg_params_order = 0x%x, label->value = 0x%"PRIx64"\n", reg_params_order[m], label->value);
						//if ((label->scope == 2) &&
						//	(label->type == 1)) {
						if (tmp_state > 0) {
							fprintf(fd, ", ");
						}
						//fprintf(fd, "int%"PRId64"_t ",
						//	label->size_bits);
						tmp = label_to_string(label, buffer, 1023);
						tmp = fprintf(fd, "%s", buffer);
						tmp_state++;
					//	}
					}
#if 0
					for (n2 = 0; n2 < external_entry_points[l].params_size; n2++) {
						struct label_s *label;
						label = &labels[external_entry_points[l].params[n2]];
						if ((label->scope == 2) &&
							(label->type == 1)) {
							continue;
						}
						if (tmp_state > 0) {
							fprintf(fd, ", ");
						}
						fprintf(fd, "int%"PRId64"_t ",
						label->size_bits);
						tmp = output_label(label, fd);
						tmp_state++;
					}
#endif
					tmp = fprintf(fd, ");%s", cr);
				} else {
					tmp = fprintf(fd, "CALL1()%s", cr);
				}
#if 0
				/* FIXME: JCD test disabled */
				call = inst_log1->extension;
				if (call) {
					for (l = 0; l < call->params_size; l++) {
						if (l > 0) {
							fprintf(fd, ", ");
						}
						label = &labels[call->params[l]];
						tmp = output_label(label, fd);
					}
				}
#endif
				//tmp = fprintf(fd, ");\n");
				//debug_print(DEBUG_OUTPUT, 1, "%s();\n",
				//	external_entry_points[instruction->srcA.index].name);
			} else {
				/* A indirect call via a function pointer or call table. */
				tmp = fprintf(fd, "(*");
				tmp = label_redirect[inst_log1->value1.indirect_value_id].redirect;
				label = &labels[tmp];
				tmp = label_to_string(label, buffer, 1023);
				tmp = fprintf(fd, "%s", buffer);
				tmp = fprintf(fd, ") ()%s", cr);
			}
//			tmp = fprintf(fd, "/* call(); */\n");
//			debug_print(DEBUG_OUTPUT, 1, "/* call(); */\n");
			break;

		case CMP:
			/* Don't do anything for this instruction. */
			/* only does anything if combined with a branch instruction */
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			debug_print(DEBUG_OUTPUT, 1, "//\tcmp ");
			tmp = fprintf(fd, "//\tcmp ");
			if (1 == instruction->srcB.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1->value2.indirect_value_id;
			} else {
				value_id = inst_log1->value2.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value3.value_id);
			tmp = fprintf(fd, " - ");
			debug_print(DEBUG_OUTPUT, 1, "\nstore=%d\n", instruction->srcA.store);
			if (1 == instruction->srcA.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1->value1.indirect_value_id;
			} else {
				value_id = inst_log1->value1.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value1.value_id);
			tmp = fprintf(fd, ";%s",cr);
			break;

		case ICMP:
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			debug_print(DEBUG_OUTPUT, 1, "\t");
			tmp = fprintf(fd, "\t");
			if (1 == instruction->dstA.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1->value3.indirect_value_id;
			} else {
				value_id = inst_log1->value3.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			//tmp = fprintf(fd, "0x%x:", tmp);
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value3.value_id);
			tmp = fprintf(fd, " = ");
			debug_print(DEBUG_OUTPUT, 1, "\nstore=%d\n", instruction->srcA.store);
			debug_print(DEBUG_OUTPUT, 1, "icmp\n");
			err = if_expression( instruction->predicate, inst_log1, label_redirect, labels, fd);
			debug_print(DEBUG_OUTPUT, 1, "prev flags=%d\n",inst_log1->instruction.flags);
			debug_print(DEBUG_OUTPUT, 1, "prev opcode=0x%x\n",inst_log1->instruction.opcode);
			debug_print(DEBUG_OUTPUT, 1, "0x%"PRIx64":%s\n", instruction->srcA.index, condition_table[instruction->predicate]);
			tmp = fprintf(fd, ";%s",cr);
			break;

		case TEST:
			/* Don't do anything for this instruction. */
			/* only does anything if combined with a branch instruction */
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			debug_print(DEBUG_OUTPUT, 1, "//\ttest ");
			tmp = fprintf(fd, "//\ttest ");
			if (1 == instruction->dstA.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1->value2.indirect_value_id;
			} else {
				value_id = inst_log1->value2.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value3.value_id);
			tmp = fprintf(fd, " , ");
			debug_print(DEBUG_OUTPUT, 1, "\nstore=%d\n", instruction->srcA.store);
			if (1 == instruction->srcA.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1->value1.indirect_value_id;
			} else {
				value_id = inst_log1->value1.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value1.value_id);
			tmp = fprintf(fd, ";%s",cr);
			break;

		case IF:
			/* FIXME: Never gets here, why? */
			/* Don't do anything for this instruction. */
			/* only does anything if combined with a branch instruction */
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			debug_print(DEBUG_OUTPUT, 1, "\t");
			tmp = fprintf(fd, "\t");
			debug_print(DEBUG_OUTPUT, 1, "if1 ");
			tmp = fprintf(fd, "if1 ");
			found = 0;
			tmp = 30; /* Limit the scan backwards */
			l = inst_log1->prev[0];
			do {
				inst_log1_flags =  &inst_log_entry[l];
				debug_print(DEBUG_OUTPUT, 1, "Previous opcode 0x%x\n", inst_log1_flags->instruction.opcode);
				debug_print(DEBUG_OUTPUT, 1, "Previous flags 0x%x\n", inst_log1_flags->instruction.flags);
				if (1 == inst_log1_flags->instruction.flags) {
					found = 1;
				}
				debug_print(DEBUG_OUTPUT, 1, "Previous flags instruction size 0x%x\n", inst_log1_flags->prev_size);
				if (inst_log1_flags->prev > 0) {
					l = inst_log1_flags->prev[0];
				} else {
					l = 0;
				}
				tmp--;
			} while ((0 == found) && (0 < tmp) && (0 != l));
			if (found == 0) {
				debug_print(DEBUG_OUTPUT, 1, "Previous flags instruction not found. found=%d, tmp=%d, l=%d\n", found, tmp, l);
				return 1;
			} else {
				debug_print(DEBUG_OUTPUT, 1, "Previous flags instruction found. found=%d, tmp=%d, l=%d\n", found, tmp, l);
			}

			err = if_expression( instruction->srcA.index, inst_log1_flags, label_redirect, labels, fd);
			debug_print(DEBUG_OUTPUT, 1, "\t prev flags=%d, ",inst_log1_flags->instruction.flags);
			debug_print(DEBUG_OUTPUT, 1, "\t prev opcode=0x%x, ",inst_log1_flags->instruction.opcode);
			debug_print(DEBUG_OUTPUT, 1, "\t 0x%"PRIx64":%s", instruction->srcA.index, condition_table[instruction->srcA.index]);
			debug_print(DEBUG_OUTPUT, 1, "\t LHS=%d, ",inst_log1->prev[0]);
			debug_print(DEBUG_OUTPUT, 1, "IF goto label%04"PRIx32";\n", inst_log1->next[1]);
			if (err) {
				debug_print(DEBUG_OUTPUT, 1, "IF CONDITION unknown\n");
				return 1;
			}
			tmp = fprintf(fd, "IF goto ");
//			for (l = 0; l < inst_log1->next_size; l++) {
//				tmp = fprintf(fd, ", label%04"PRIx32"", inst_log1->next[l]);
//			}
			tmp = fprintf(fd, "label%04"PRIx32";", inst_log1->next[1]);
			tmp = fprintf(fd, "%s", cr);
			tmp = fprintf(fd, "\telse goto label%04"PRIx32";%s", inst_log1->next[0], cr);

			break;

		case BC:
			/* FIXME: Never gets here, why? */
			/* Don't do anything for this instruction. */
			/* only does anything if combined with a branch instruction */
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			debug_print(DEBUG_OUTPUT, 1, "\t");
			tmp = fprintf(fd, "\t");
			debug_print(DEBUG_OUTPUT, 1, "if (");
			tmp = fprintf(fd, "if (");
//			debug_print(DEBUG_OUTPUT, 1, "\t prev flags=%d, ",inst_log1_flags->instruction.flags);
//			debug_print(DEBUG_OUTPUT, 1, "\t prev opcode=0x%x, ",inst_log1_flags->instruction.opcode);
//			debug_print(DEBUG_OUTPUT, 1, "\t LHS=%d, ",inst_log1->prev[0]);
//			debug_print(DEBUG_OUTPUT, 1, "IF goto label%04"PRIx32";\n", inst_log1->next[1]);
			if (1 == instruction->srcA.indirect) {
				tmp = fprintf(fd, "*");
				value_id = inst_log1->value1.indirect_value_id;
			} else {
				value_id = inst_log1->value1.value_id;
			}
			tmp = label_redirect[value_id].redirect;
			label = &labels[tmp];
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			tmp = fprintf(fd, ")");
			debug_print(DEBUG_OUTPUT, 1, "\nstore=%d\n", instruction->srcA.store);
			tmp = fprintf(fd, " goto ");
//			for (l = 0; l < inst_log1->next_size; l++) {
//				tmp = fprintf(fd, ", label%04"PRIx32"", inst_log1->next[l]);
//			}
			tmp = fprintf(fd, "label%04"PRIx32";", inst_log1->next[1]);
			tmp = fprintf(fd, "%s", cr);
			tmp = fprintf(fd, "\telse goto label%04"PRIx32";%s", inst_log1->next[0], cr);

			break;

		case NOP:
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			break;
		case RET:
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			debug_print(DEBUG_OUTPUT, 1, "\t");
			tmp = fprintf(fd, "\t");
			debug_print(DEBUG_OUTPUT, 1, "return\n");
			tmp = fprintf(fd, "return ");
			tmp = label_redirect[inst_log1->value1.value_id].redirect;
			label = &labels[tmp];
			tmp = label_to_string(label, buffer, 1023);
			tmp = fprintf(fd, "%s", buffer);
			//tmp = fprintf(fd, " /*(0x%"PRIx64")*/", inst_log1->value1.value_id);
			tmp = fprintf(fd, ";%s", cr);
			break;
		default:
			debug_print(DEBUG_OUTPUT, 1, "Unhandled output instruction1 opcode=0x%x\n", instruction->opcode);
			tmp = fprintf(fd, "//Unhandled output instruction\\l");
			if (print_inst(self, instruction, inst_number, labels))
				return 1;
			return 1;
			break;
		}
		if (0 < inst_log1->next_size && inst_log1->next[0] != (inst_number + 1)) {
			debug_print(DEBUG_OUTPUT, 1, "\tTMP3 goto label%04"PRIx32";\n", inst_log1->next[0]);
			tmp = fprintf(fd, "\tTMP3 goto label%04"PRIx32";%s", inst_log1->next[0], cr);
		}
	}
	return 0;
}

int output_function_body(struct self_s *self, struct process_state_s *process_state,
			 FILE *fd, int start, int end, struct label_redirect_s *label_redirect, struct label_s *labels)
{
	int tmp, n;

	if (!start || !end) {
		debug_print(DEBUG_OUTPUT, 1, "output_function_body:Invalid start or end\n");
		return 1;
	}
	debug_print(DEBUG_OUTPUT, 1, "output_function_body:start=0x%x, end=0x%x\n", start, end);

	for (n = start; n <= end; n++) {
		tmp = output_inst_in_c(self, process_state, fd, n, label_redirect, labels, "\n");
	}
#if 0
	if (0 < inst_log1->next_size && inst_log1->next[0]) {
		debug_print(DEBUG_OUTPUT, 1, "\tTMP1 goto label%04"PRIx32";\n", inst_log1->next[0]);
		tmp = fprintf(fd, "\tTMP1 goto label%04"PRIx32";\n", inst_log1->next[0]);
	}
#endif
	tmp = fprintf(fd, "}\n\n");
	return 0;
}

