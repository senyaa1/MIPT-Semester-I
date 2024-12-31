#include <stdlib.h>
#include <stdio.h>

#include "parser.h"
#include "tex.h"

#define TEX_OP(OP_NAME, OP)					\
	case OP_NAME:						\
		tex_dump_recursive(writer, node->left);		\
		bufcpy(writer, OP);				\
		tex_dump_recursive(writer, node->right);	\
		break;						\

#define TEX_FUNC(FUNC_NAME, FUNC)				\
	case FUNC_NAME:						\
		bufcpy(writer, FUNC "(");			\
		tex_dump_recursive(writer, node->left);	\
		bufcpy(writer, ")");				\
		break;						\


static diff_node_t* tex_dump_recursive(buf_writer_t* writer, diff_node_t* node)
{
	if(!node) return 0;

	char* num_str = 0;
	switch(node->type)
	{
		case NODE_VARIABLE:
			bufcpy(writer, node->value.variable);
			break;
		case NODE_NUMBER:
			asprintf(&num_str, "%g", node->value.number);
			bufcpy(writer, num_str);
			free(num_str);
			break;
		case NODE_FUNCTION:
			switch(node->value.op_type)
			{
				TEX_OP(ADD, " + ")
				TEX_OP(SUB, " - ")
				TEX_OP(MUL, " \\cdot ")
				TEX_FUNC(SIN, "\\sin")
				TEX_FUNC(COS, "\\cos")
				TEX_FUNC(TG, "\\tg")
				TEX_FUNC(CTG, "\\ctg")
				TEX_FUNC(LN, "\\ln")
				TEX_FUNC(ARCSIN, "\\arcsin")
				TEX_FUNC(ARCCOS, "\\arccos")
				TEX_FUNC(ARCTG, "\\arctg")
				TEX_FUNC(ARCCTG, "\\arcctg")

				TEX_FUNC(SH, "\\sh")
				TEX_FUNC(CH, "\\ch")
				TEX_FUNC(TH, "\\th")
				TEX_FUNC(CTH, "\\cth")

				case DIV:
					bufcpy(writer, "\\frac{");
					tex_dump_recursive(writer, node->left);
					bufcpy(writer, "}{");
					tex_dump_recursive(writer, node->right);
					bufcpy(writer, "}");
					break;
				case POW:
					if(node->left->type == NODE_FUNCTION)
						bufcpy(writer, "(");
					bufcpy(writer, "{");
					tex_dump_recursive(writer, node->left);
					if(node->left->type == NODE_FUNCTION)
						bufcpy(writer, ")");
					bufcpy(writer, "}^{");
					tex_dump_recursive(writer, node->right);
					bufcpy(writer, "}");
					break;
				case LOG:
					bufcpy(writer, "\\log_(");
					tex_dump_recursive(writer, node->left);
					bufcpy(writer, "}(");
					tex_dump_recursive(writer, node->right);
					bufcpy(writer, "}");
					break;
				case SQRT:
					bufcpy(writer, "\\sqrt{");
					tex_dump_recursive(writer, node->left);
					bufcpy(writer, "}");
					break;
			}
			break;
		default:
			return 0;
	}
	
	return 0;
}

void tex_dump_equation(buf_writer_t* writer, diff_node_t* node, char* preamble)
{
	bufcpy(writer, "\n\\begin{dmath*}\n");

	if(preamble) bufcpy(writer, preamble);

	tex_dump_recursive(writer, node);
	bufcpy(writer, "\n\\end{dmath*}\n");
}

buf_writer_t tex_init(diff_node_t* tree)
{
	buf_writer_t writer = { writer.buf = (char*)calloc(DEFAULT_TEX_ALLOC, sizeof(char)), .buf_len = DEFAULT_TEX_ALLOC};

	bufcpy(&writer, "\\documentclass[12pt,a4paper]{extreport}\n");
	bufcpy(&writer, "\\input{style}\n");
	bufcpy(&writer, "\\title{<<Самостоятельная работа по вычислению производных высших порядков>>}\n");
	bufcpy(&writer, "\\begin{document}\n");
	bufcpy(&writer, "\\maketitle\n");
	bufcpy(&writer, "\\pagebreak\n");
	bufcpy(&writer, "\\tableofcontents\n");
	bufcpy(&writer, "\\pagebreak\n");
	bufcpy(&writer, "\\section{Вступление}\n");

	bufcpy(&writer, "В первом классе советской школы математика была не просто предметом, а боевым рубежом. Пока загнивающий Запад в детских садах изучал цвета радуги и делал поделки из макарон, наши первоклассники уже знали, что дифференцировать функции — это не прихоть, а вопрос государственной важности. С урока сразу на доске красовалось грозное: “ДЕРИВАТЫ — старшие братья численных рядов!”. Мелом, быстро и четко. \n\nУчительница Мария Ивановна, с легким прищуром и неотразимой верой в светлое будущее, объясняла суть производной на примере сбора картошки: “Если Ваня копает одну сотку за 10 минут, а Петя — за 5 минут, то чья производная выше?”. Кто не понимал, оставался после уроков считать частные производные по полям кукурузы.\n\n\n\nЗато к концу первой четверти маленькие дифференциаторы могли находить скорость распространения слухов в очереди за колбасой, а на переменах спорили о втором законе Ньютона, пока взрослые стояли в очереди за учебниками. Такие времена, такой уровень. И если кто-то на вопрос “Чему равна производная синуса?” пытался сказать “Что такое синус?”, его тут же отправляли в третий класс — в народное хозяйство стране помощники нужны!");


	return writer;

}

char* tex_end(buf_writer_t* writer)
{
	bufcpy(writer, "\\end{document}\n");
	writer->buf = (char*)realloc(writer->buf, (writer->cursor + 1)* sizeof(char));
	writer->buf[writer->cursor] = '\x00';
	return writer->buf;
}
