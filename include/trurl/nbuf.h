/* 
  TRURLib
  Dynamic buffer
  $Id$
*/

#ifndef TRURL_BUF_H
#define TRURL_BUF_H

#define TN_BUF_CONSTSIZE         (1 << 0)
#define TN_BUF_CONSTDATA         (1 << 1)

typedef struct trurl_buf tn_buf;

tn_buf *n_buf_new(int size);
tn_buf *n_buf_init(tn_buf *buf, void *buffer, int size);
tn_buf *n_buf_clean(tn_buf *buf);
void n_buf_free(tn_buf *buf);

tn_buf *n_buf_ctl(tn_buf *buf, unsigned flags);

int n_buf_add(tn_buf *buf, const void *data, int size);

int n_buf_addstring(tn_buf *buf, const char *str, int with_zero);
#define n_buf_addstr(buf, str)  n_buf_addstring(buf, str, 0)
#define n_buf_addstrz(buf, str) n_buf_addstring(buf, str, 1)

int n_buf_size(const tn_buf *buf);
void *n_buf_ptr(const tn_buf *buf); /* returns buffer data pointer */


struct trurl_buf_iterator {
    tn_buf *nbuf;
    size_t  offs;
};

typedef struct trurl_buf_iterator tn_buf_it;

void n_buf_it_init(tn_buf_it *bufi, tn_buf *buf);
void *n_buf_it_get(tn_buf_it *bufi, size_t size);

#endif /* TRURL_BUF_H */
    

        


    


    
