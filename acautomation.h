#include <stdint.h>
#include <stddef.h>
#define MAXSIZE 10000

#ifdef __cplusplus
    extern "C" {
#endif

struct TrieNode
{
    struct TrieNode *fail;
    struct TrieNode **children;
    char *pattern;
    size_t children_len;
    size_t pattern_len;
    int32_t end;
	char * wholeWord;
};

struct ACAutoMation
{
    struct TrieNode *root;
    int32_t dirty_node;
    int32_t (*get_word_length)(const char *str, size_t length); // the right length of a (chinese) word in (utf-8).
	int32_t (*get_index)(struct TrieNode** children,size_t children_len, const char *str,size_t str_length); // get index of the part of the pattern word
    int32_t word_count;
};

struct TrieNode **que;
int que_left,que_right;
struct TrieNode* queue(int type, struct TrieNode *input);
int isfind;

struct ACAutoMation *ac_create();

void ac_add(struct ACAutoMation *mation, const char *pattern, size_t length);

void ac_destroy(struct ACAutoMation *mation);

int32_t ac_match(struct ACAutoMation *mation, const char *str, size_t length);


#ifdef __cplusplus
    }
#endif
