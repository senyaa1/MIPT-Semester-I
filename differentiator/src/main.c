#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "graph.h"
#include "lexer.h"
#include "parser.h"
#include "differentiator.h"
#include "tex.h"
#include "fs.h"
#include "color.h"
#include "plot.h"

// #define DRAW_GRAPH


int main()
{
	printf(WHITE "Enter expression> " BLUE);
	char* str_expression = (char*)calloc(INPUT_BUF_SZ, sizeof(char));
	fgets(str_expression, INPUT_BUF_SZ, stdin);
	printf(WHITE "Count of derivatives: " BLUE );

	// const char *str_expression = "((2 + 5) * x * (x - 4) / (300 * x + (100 / (400 / (x - 10000 / x^2 - 5 * x)))) * ((2 + 5) * x * (x - 4) / (300 * x + (100 / (400 / (x - 10000 / x^2 - 5 * x)))) + 1337";
	// const char *str_expression = "sin(15 * x ^ 3) + cos (20 * x + 1) ^ 3";
	// const char *str_expression = "2 * x + 5 * x + 4 * x ^2 + 10 * x ^ 3";

	int deriv_cnt = 4; 
	scanf("%d", &deriv_cnt);

	printf(RESET);


	char fmtbuf[256] = { 0 };
	lexer_t* lexer = lex(str_expression);
	diff_node_t* expression = parse_expression(lexer);

	#ifdef DRAW_GRAPH
	draw_tree(expression, "out/normal.png");
	#endif 

	buf_writer_t writer = tex_init(expression);
	bufcpy(&writer, "\\section{Вычисление}\n");
	bufcpy(&writer, "\n\nДавайте продифференцируем данную легчайшую функцию.\n");
	tex_dump_equation(&writer, expression, "f(x) = ");

	diff_node_t* diff_tree = node_copy(expression);
	for(int i = 1; i <= deriv_cnt; i++)
	{
		sprintf(fmtbuf, "\nВычислим %d-ую производную:\n", i);
		bufcpy(&writer, fmtbuf);
		sprintf(fmtbuf, "f^{(%d)}(x) = ", i);

		diff_node_t* new_diff_tree = differentiate(diff_tree, &writer);
		tex_dump_equation(&writer, new_diff_tree, fmtbuf);
		new_diff_tree = optimize(new_diff_tree, &writer);

		sprintf(fmtbuf, "\n\nПолучаем %d-ую производную:\n", i);
		bufcpy(&writer, fmtbuf);

		sprintf(fmtbuf, "f^{(%d)}(x) = ", i);
		tex_dump_equation(&writer, new_diff_tree, fmtbuf);
		
		node_free(diff_tree);
		diff_tree = new_diff_tree;
	}


	bufcpy(&writer, "\\section{Заключение}\n");
	bufcpy(&writer, "Наша функция и полученная производная:\n\n");
	tex_dump_equation(&writer, expression, "f(x) = ");

	sprintf(fmtbuf, "f^{(%d)}(x) = ", deriv_cnt);
	tex_dump_equation(&writer, diff_tree, fmtbuf);

	#ifdef DRAW_GRAPH
	draw_tree(diff_tree, "out/optimized.png");
	#endif

	bufcpy(&writer, "\n\nНесложно заметить, что графики выглядят так:\n\n");
	plot_dump(expression, "функция", "blue", "out/orig_plot.png");
	plot_dump(diff_tree, "производная", "orange", "out/optimized_plot.png");
	bufcpy(&writer, "\\begin{minipage}{0.45\\textwidth}\n");
	bufcpy(&writer, "\\centering");
	bufcpy(&writer, "\\includegraphics[width=\\linewidth]{out/orig_plot.png}");
	bufcpy(&writer, "\\end{minipage}\n");
	bufcpy(&writer, "\\hfill\n");
	bufcpy(&writer, "\\begin{minipage}{0.45\\textwidth}\n");
	bufcpy(&writer, "\\centering");
	bufcpy(&writer, "\\includegraphics[width=\\linewidth]{out/optimized_plot.png}");
	bufcpy(&writer, "\\end{minipage}\n");

	char* tex = tex_end(&writer);
	write_file("out/dump.tex", tex, strlen(tex));
	system("latexmk -f out/dump.tex");

	free(writer.buf);
	node_free(expression);
	node_free(diff_tree);
	free_lexer(lexer);
	free(str_expression);

	return 0;
}
