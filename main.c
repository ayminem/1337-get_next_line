#include "get_next_line.h"

int	main(void)
{
	char	*r;
	char	*s;

	puts("--- ft_strlen");
	check(ft_strlen(NULL) == 0, "strlen(NULL) == 0");
	check(ft_strlen("") == 0, "strlen(\"\") == 0");
	check(ft_strlen("abc\n") == 4, "strlen(\"abc\\n\") == 4");

	puts("--- ft_strchr");
	check(ft_strchr(NULL, '\n') == NULL, "strchr(NULL, '\\n') == NULL");
	check(ft_strchr("", '\n') == NULL, "strchr(\"\", '\\n') == NULL");
	check(ft_strchr("abc", '\n') == NULL, "no newline -> NULL");
	s = "ab\ncd\n";
	check(ft_strchr(s, '\n') == s + 2, "returns the FIRST newline");
	check(ft_strchr("\n", '\n') != NULL, "only a newline");

	puts("--- ft_strjoin (stash is freed inside)");
	r = ft_strjoin(NULL, "abc");
	check(same(r, "abc"), "join(NULL, \"abc\") == \"abc\"");
	r = ft_strjoin(r, "def");
	check(same(r, "abcdef"), "join(\"abc\", \"def\") == \"abcdef\"");
	r = ft_strjoin(r, "");
	check(same(r, "abcdef"), "join(s, \"\") == s");
	free(r);
	r = ft_strjoin(dup(""), "");
	check(same(r, ""), "join(\"\", \"\") == \"\"");
	free(r);
	r = ft_strjoin(dup("ab\nc"), "de\nf");
	check(same(r, "ab\ncde\nf"), "join keeps newlines");
	free(r);

	puts("--- ft_substr");
	r = ft_substr("hello", 1, 3);
	check(same(r, "ell"), "substr(\"hello\", 1, 3) == \"ell\"");
	free(r);
	r = ft_substr("hello", 0, 100);
	check(same(r, "hello"), "len larger than the string is clamped");
	free(r);
	r = ft_substr("hello", 5, 3);
	check(same(r, ""), "start == strlen -> \"\"");
	free(r);
	r = ft_substr("hello", 10, 3);
	check(same(r, ""), "start > strlen -> \"\"");
	free(r);
	r = ft_substr("hello", 2, 0);
	check(same(r, ""), "len 0 -> \"\"");
	free(r);
	check(ft_substr(NULL, 0, 3) == NULL, "substr(NULL, ...) == NULL");

	printf("\n%s (%d failure%s)\n", g_fail ? "SOME TESTS FAILED" : "ALL PASSED",
		g_fail, g_fail == 1 ? "" : "s");
	return (g_fail != 0);
}