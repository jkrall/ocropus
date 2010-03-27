/*
  $Date: 1999/10/15 12:40:27 $
  $Revision: 1.1.1.1 $
  $Author: kise $
  hash.c
  ハッシュ関数

  key : 2つの連結成分のラベルに対し, idの値を求める関数
  search : id の値がハッシュ表に既に登録されているかを調べる関数
  enter : 登録されていないid をハッシュ表に登録する関数
*/

#include <stdio.h>
#include "const.h"
#include "defs.h"
#include "extern.h"
#include "function.h"


namespace voronoi{
    /* ハッシュ関数 1 */
    HashVal hash1(Key key)
    {
        return(key % M1);
    }

    /* ハッシュ関数 2 */
    HashVal hash2(Key key)
    {
        return(key % M2);
    }

    /* 初期化関数 */
    void init_hash()
    {
        HashVal i;

        for(i=0;i<M1+M2;i++)
            hashtable[i]=NIL;
    }

    /*
     * ハッシュ関数に対するキーをつくる関数
     * 2つのラベルを受けとり, それに対するid を返す
     */
    Key key(Label lab1, Label lab2)
    {
        unsigned long key,tmp;

        key = lab1;
        key = key << 2*BYTE;
        tmp = lab2;
        key = key | tmp;
        return key;
    }

    /* id がハッシュ表に登録されているかを調べる関数 */ 
    int search(Label lab1, Label lab2)
    {
        Key id;
        HashVal x;
        HashTable *p;

        id = key(lab1,lab2);
        x = hash1(id)+hash2(id);	/* ハッシュ値を計算 */
        p = hashtable[x];

        while(p != NIL) {
            if((lab1 == p->lab1) && (lab2 == p->lab2)) /* 登録されていると */
                return(p->entry);	                       /* そのentry の値を返す */
            p = p->next;
        }
    
        return NODATA;	/* 登録されていないときはNODATA の値を返す */
    }

    /*
     * 登録されていないid とそれに対するentry の値を
     * ハッシュ表に登録する関数
     */ 
    void enter(Label lab1, Label lab2, unsigned int entry)
    {
        Key id;
        HashVal x;
        HashTable *p;

        id = key(lab1,lab2);
        x = hash1(id)+hash2(id);	/* ハッシュ値を計算 */
    
        /* 登録するための領域を確保する */
        p = (HashTable *)myalloc(sizeof(HashTable)* 1);

        /* 確保した領域を挿入し, 値を登録する */
        p->next = hashtable[x];
        hashtable[x] = p;
        hashtable[x]->lab1 = lab1;
        hashtable[x]->lab2 = lab2;
        hashtable[x]->entry = entry;
    }
}
