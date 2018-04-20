#include <stdlib.h>
#include <string.h>

#include "acautomation.h"
//get word length
int32_t get_utf_8_word_length(const char *str, size_t length)
{
    if (length < 1) {
        return 0;
    }
    char tmp = str[0];
    if (tmp > 0) {
        return 1;
    }

    int i = 0;
    do {
        tmp <<= 1;
        i++;
    } while (tmp < 0);

    return i > length? length : i;
}

int32_t get_child_index(struct TrieNode **children,size_t children_len,const char *str,size_t str_length) {
	int  left = 0,right = children_len-1;
	int  mid = (left+right)/2;
	while(left <= right) {
		struct TrieNode *current = children[mid];
		if(strncmp(current->pattern,str,str_length) == 0) {
			return mid;
		}	
		else if(strncmp(current->pattern,str,str_length) >0 )
			right = mid-1;
		else
			left = mid +1;
		mid = (left+right)/2;
	}
	return -1;
}

struct ACAutoMation *ac_create()
{
    struct ACAutoMation *mation = (struct ACAutoMation *)malloc(sizeof(struct ACAutoMation));
	que = (struct TrieNode **)malloc(sizeof(struct TrieNode)*1000);
	que_left = que_right = 0;
    if (NULL == mation) {
        return NULL;
    }
    mation->root = (struct TrieNode*)malloc(sizeof(struct TrieNode));
    if (NULL == mation->root) {
        free(mation);
        return NULL;
    }

    mation->root->children = NULL;
    mation->root->children_len = 0;
    mation->get_word_length = get_utf_8_word_length;
	mation->get_index = get_child_index;
    mation->dirty_node = 0;
    mation->word_count = 0;
    return mation;
}

void destroy_trie_node(struct TrieNode *node)
{
    if (NULL == node) {
        return;
    }
    int32_t i = 0;
    for(; i < node->children_len; i++) {
        destroy_trie_node(node->children[i]);
    }
    free(node->children);
    free(node->pattern);
    free(node);
}

void ac_destroy(struct ACAutoMation *mation)
{
    if (NULL == mation) {
        return;
    }
    destroy_trie_node(mation->root);
    free(mation);
}

void add_trie_node(struct ACAutoMation *mation, struct TrieNode *parent, const char *str, size_t length, const char *pattern)
{
    struct TrieNode *node = NULL;
    size_t pattern_len = (*(mation->get_word_length))(str, length);
    if (NULL != parent->children) {
		 int index = mation->get_index(parent->children,parent->children_len,str,pattern_len);
		 if(index != -1)
				node = parent->children[index];
       }

    if (NULL == node) {
        node = (struct TrieNode*)malloc(sizeof(struct TrieNode));
        if (NULL == node) {
            return;
        }
        node->pattern = (char*)malloc(pattern_len);
        if (NULL == node->pattern) {
            free(node);
            return;
        }
        node->children = NULL;
        node->children_len = 0;
        node->end = 0;
        node->pattern_len = pattern_len;
		node->wholeWord = NULL;

        struct TrieNode **children = realloc(parent->children, sizeof(struct TrieNode *) * (parent->children_len+1)*2);
        if (NULL == children) {
            free(node->pattern);
            free(node);
            return;
        }
		int32_t children_len = parent->children_len + 1;
        parent->children_len = children_len;
        parent->children = children;

        memcpy(node->pattern, str, node->pattern_len);
		int k = 0,i,flag = 0;
		for(i=0; i<children_len-1; i++) {	
			if(strncmp(parent->children[i]->pattern,str, pattern_len) > 0){
				k = i;
				flag = 1;
				break;
			}
		}
		if(flag) {
			for(i=children_len-1;i>k;i--) 
				parent->children[i] = parent->children[i-1];
			parent->children[k] = node;
		}
		else {	
			parent->children[children_len-1] = node;
		}
        node->fail = parent;
        mation->dirty_node += 1;
    }

    if (node->pattern_len == length) { // end
        node->end += 1;
        mation->word_count += 1;
		node->wholeWord = (char *)malloc(strlen(pattern)); 
		strcpy(node->wholeWord, pattern);
    } else {
        add_trie_node(mation, node, str + node->pattern_len, length - node->pattern_len, pattern);
    }
}

void ac_add(struct ACAutoMation *mation, const char *str, size_t length)
{
    add_trie_node(mation, mation->root, str, length, str);
}

void build_failed_pointer(struct ACAutoMation *mation) {
	struct TrieNode *root = mation->root;
	if(root == NULL) {
		return ;
	}	
	int i = 0;
	for(; i < root->children_len; i++) {
		root->children[i]->fail = root;
		queue(1, root->children[i]);
	}
	while(que_left < que_right) {
		struct TrieNode *out = queue(0, NULL);
		for( i = 0; i < out->children_len; i++ ) {
			struct TrieNode *fail = out->fail;
			struct TrieNode *tmp = out->children[i];
			while(fail) {
				int index = mation->get_index(fail->children, fail->children_len, tmp->pattern, tmp->pattern_len);
				if(index == -1) {
					fail = fail->fail;
				} else{
					tmp->fail = fail->children[index];	
					break;
				}
			}
			if(!fail) {
				tmp->fail = root;
			}
			queue(1, tmp);
		}
	}
}


struct TrieNode* queue(int type, struct TrieNode *input ) {
	if(type == 1) {
		if(que_right + 1 == que_left || (que_left == 0 && que_right == MAXSIZE-1)) {
			printf("queue full");
			return NULL;
		}
		if(!que[que_right]) {
			struct TrieNode *newNode = (struct TrieNode*)malloc(sizeof(struct TrieNode));
			if(!newNode) {
				printf("queue get memory failed!\n");
				return NULL;
			}
			if(que_right == MAXSIZE-1) {
				que_right = 0;
			}
		}
		que[que_right] = input; 
		que_right++;
	} else {
		if(que_left > que_right || !que[que_left]) {
			printf("queue no data!\n");
			return NULL;
		}		
		struct TrieNode *ret = que[que_left];
		if(que_left == MAXSIZE-1) {
			que_left = 0;
		}
		que_left++;
		return ret; 
	}
}

int32_t match_trie_node(struct ACAutoMation *mation, struct TrieNode* current, const char *str, size_t length)
{
 	if(length == 0)	{
		return 1;	
	}
    size_t pattern_len = (*(mation->get_word_length))(str, length);
    int index = mation->get_index(current->children,current->children_len,str,pattern_len);
	if(index != -1) {
		struct TrieNode* children = current->children[index];
		if(children->end > 0) {
			isfind = 1;
			printf("%s\n", children->wholeWord);
		}
		return match_trie_node(mation, children, str + children->pattern_len, length - children->pattern_len);
     }

    if(current != mation->root) {
        return match_trie_node(mation, current->fail, str, length);
    }

    // not matched
    if (pattern_len <= length && pattern_len > 0) {
		return match_trie_node(mation, current, str + pattern_len, length - pattern_len);
    }
    return -1;
}

int ac_match(struct ACAutoMation *mation, const char *str, size_t length)
{
    if (mation->dirty_node > 0) {
        build_failed_pointer(mation);
        mation->dirty_node = 0;
    }

	isfind = 0;
    match_trie_node(mation, mation->root, str, length);
	return isfind ? 1 : 0;
}
