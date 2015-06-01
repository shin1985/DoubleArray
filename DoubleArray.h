//============================================================================
// Name        : DoubleArray.h
// Author      : Shingo.Akiyoshi
// Version     : 0.1(2008)
// Copyright   : master@akishin.netまでお問い合わせください。
// Description : DoubleArray in C, Ansi-style => インターフェースだけC++へ変更(2008.2.14)
// Memo        : メモリ削減のため、S-JIS前提の実装してます。UTF-8にしないこと！
//============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <locale.h>
#include <sys/stat.h>

#define SIZE_T 128
#define SIZE_CHAR 256

/* ダブル配列のクラス */
class DoubleArrayLib{
private:
	/* BASE,CHECKのデータ構造 */
	typedef struct{
		int base;
		int check;
	}BC;

	struct resolve_elm {
		int c;
		int base;
	};
	struct resolve_stat {
		int nr;
		struct resolve_elm elm[SIZE_CHAR];
	};
private:
	/* ダブル配列の宣言 */
	BC *m_ptr_bc;
	/* 現在のダブル配列のサイズ */
	int m_size;
public:
	/* コンストラクタ */
	DoubleArrayLib();
	/* その文字列が存在するか */
	int search_str(const wchar_t *str, int len);
	/* 文字列を追加 */
	void add_str(const wchar_t *str, int len);
	/* 登録一覧 */
	void da_dump();
	void da_traverse(int idx, wchar_t *buf, int len);
private:
	/* ダブル配列のサイズ拡張 */
	void change_array_size(int size);
	/* ノードを追加(文字単位) */
	int add_node(int idx, wchar_t c);
	int get_base(int idx);
	int get_check(int idx);
	void set_base(int idx, int base);
	void set_check(int idx, int check);
	int resolve_conflict(int parent, wchar_t c);
	void collect_child(struct resolve_stat *rs, int parent, wchar_t c);
	int find_place(struct resolve_stat *rs);
	void move_children(struct resolve_stat *rs, int parent, int left);
	void push_child(struct resolve_stat *rs, int c, int orig_base);
};

DoubleArrayLib::DoubleArrayLib(){
  this->m_size = 0;
  this->m_ptr_bc = NULL;
  this->set_base(1, 1);
  this->set_check(1, 1);
}

void DoubleArrayLib::change_array_size(int size){
  if (size > this->m_size) {
    this->m_ptr_bc = (BC *)realloc(this->m_ptr_bc, size * sizeof(BC));
	for (int i = this->m_size; i < size; i++) {
		this->m_ptr_bc[i].base = 0;
		this->m_ptr_bc[i].check = 0;
    }
    this->m_size = size;
  }
}

void DoubleArrayLib::set_base(int idx, int base){
  this->change_array_size(idx + 1);
  this->m_ptr_bc[idx].base = base;
}


void DoubleArrayLib::set_check(int idx, int check){
  this->change_array_size(idx + 1);
  this->m_ptr_bc[idx].check = check;
}

int DoubleArrayLib::get_base(int idx){
  this->change_array_size(idx + 1);
  return this->m_ptr_bc[idx].base;
}

int DoubleArrayLib::get_check(int idx){
  this->change_array_size(idx + 1);
  return this->m_ptr_bc[idx].check;
}

int DoubleArrayLib::search_str(const wchar_t *str, int len){
  int cur = 1;
  //printf("%d登録",this->m_size);
  for (int i = 0; i < len; i++) {
    int base = get_base(cur);
    wchar_t c = str[i];
    if (this->get_check(cur + base + c) != cur) {
      /* not match */
      return 0;
    }
    cur = cur + base + c;
  }
  return cur;
}

void DoubleArrayLib::add_str(const wchar_t *str, int len){
  int prev_idx = 1;
  for (int i = 0; i < len && str[i]!=13 && str[i]!=10; i++) {
    int idx = this->search_str((const wchar_t *)str, i + 1);
    if (idx == 0) {
      idx = this->add_node(prev_idx, str[i]);
    }
    prev_idx = idx;
  }
}

int DoubleArrayLib::add_node(int idx, wchar_t c){
  int base = get_base(idx);
  int next = idx + base + c;
  int cur_check = this->get_check(next);
  int new_base;
  /* 既に登録済みか？ */
  if (cur_check > 0) {
    //printf("conflict(%d,%C)\n", idx, c);
	  //wprintf(L"かぶった。今の要領%d かぶった語%d\n",this->m_size, c);
    return resolve_conflict(idx, c);
  }
  this->set_check(next, idx);
  /**/
  new_base = 1;
  if (idx == 1) {
    new_base = SIZE_CHAR - next;
  }
  this->set_base(next, new_base);
  return next;
}

int DoubleArrayLib::resolve_conflict(int parent, wchar_t c){
  int left;
  struct resolve_stat rs;
  /* collect */
  collect_child(&rs, parent, c);
  /* find */
  left = find_place(&rs);
  /*printf("left=%d\n", left);*/
  /* move */
  move_children(&rs, parent, left);
  return left + c;
}

void DoubleArrayLib::collect_child(struct resolve_stat *rs, int parent, wchar_t c){
  int base = this->get_base(parent);
  rs->nr = 0;
  this->change_array_size(base + SIZE_CHAR);
  for (int i = 0; i < SIZE_CHAR; i++) {
    int idx = parent + base + i;
    if (i == c) {
      this->push_child(rs, c, 1);
    } else if (this->get_check(idx) == parent) {
      /* escape */
      /*printf("idx=%d,base=%d, parent=%d,i=%d\n",
	idx, base, parent, i);*/
      this->push_child(rs, i, this->get_base(idx));
      /* clear */
      this->set_base(idx, 0);
      this->set_check(idx, 0);
    }
  }
}

int DoubleArrayLib::find_place(struct resolve_stat *rs){
  for (int left = 2; ; left ++) {
    int conflict = 0;
	this->change_array_size(left + SIZE_CHAR);
    for (int i = 0; i < rs->nr; i++) {
      if (this->get_check(rs->elm[i].c + left) > 0) {
			conflict = 1;
      }
    }
    if (!conflict) {
      return left;
    }
  }
  return 0;
}

void DoubleArrayLib::move_children(struct resolve_stat *rs, int parent, int left){
  int i;
  int orig_base = this->get_base(parent);
  int move;
  move = parent + orig_base - left;
  /*printf("parent=%d, nr=%d, left=%d, orig_base=%d\n",
    parent, rs->nr, left, orig_base);*/
  this->set_base(parent, left - parent);
  /* put moved children */
  for (i = 0; i < rs->nr; i++) {
    int idx = left + rs->elm[i].c;
    this->set_check(idx, parent);
    this->set_base(idx, rs->elm[i].base + move);
  }
  /* trim grand children */
  for (i = 0; i < rs->nr; i++) {
    int orig_left, old_child;
    int j;
    /**/
    orig_left = parent + orig_base + rs->elm[i].base + rs->elm[i].c;
    old_child = parent + orig_base + rs->elm[i].c;
    /* printf("parent=%d, old_child=%d, %d\n", parent, old_child, move); */
    for (j = 0; j < SIZE_CHAR; j++) {
      int orig_check = this->get_check(orig_left + j);
      if (this->get_check(orig_left + j) == old_child &&
	  this->get_check(orig_check) == 0) {
	int new_check;
	new_check = this->get_check(orig_left +j) - move;
	/*printf("rewrite check[%d] %d to %d (%d)\n",
	       orig_left + j,
	       get_check(da, orig_left + j), new_check,
	       get_check(da, orig_check));*/
	this->set_check(orig_left + j, new_check);
      }
    }
  }
}

void DoubleArrayLib::push_child(struct resolve_stat *rs, int c, int orig_base){
  int n = rs->nr;
  rs->nr++;
  /*printf("c=%c(%d), orig_base=%d\n", c, c, orig_base);*/
  rs->elm[n].c = c;
  rs->elm[n].base = orig_base;
}

void DoubleArrayLib::da_dump()
{
  wprintf(L"da dump size=%d\n", this->m_size);
  for (int i = 0; i < this->m_size; i++) {
    if (get_check(i)) {
      wprintf(L"%d: base=%d,check=%d\n", i,
	     get_base(i),
	     get_check(i));
    }
  }
  wprintf(L"da dump done\n");
}

void DoubleArrayLib::da_traverse(int idx, wchar_t *buf, int len){
  int base = get_base(idx);
  for (int i = 0; i < SIZE_CHAR; i++) {
    if (get_check(idx + base + i) == idx) {
      buf[len] = i;
      if (i == 0) {
		wprintf(L"find %s\n", buf);
      } else {
	da_traverse(idx + base + i, buf, len + 1);
      }
    }
  }
}
