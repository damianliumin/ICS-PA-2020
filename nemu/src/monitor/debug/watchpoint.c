#include "watchpoint.h"
#include "expr.h"
#include "monitor/monitor.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
  assert(free_ != NULL);
  if(head == NULL){
    head = free_;
    free_ = free_->next;
    head->next = NULL;
    return head;
  } else {
    WP* p = head;
    while(p->next != NULL)
      p = p->next;
    p->next = free_;
    free_ = free_->next;
    p->next->next = NULL;
    return p->next;
  }
}

void free_wp(WP* wp){
  if(wp == head){
    head = head->next;
    wp->next = free_;
    free_ = wp;
  } else {
    WP* p = head;
    while(p->next != wp)
      p = p->next;
    p->next = p->next->next;
    wp->next = free_;
    free_ = wp;
  }
  return;
}

void check_wp(){
  WP* p = head;
  while(p != NULL){
    bool success = true;
    uint32_t ret = expr(p->expression, &success);
    if(!success){
      printf("Cannot get the new value of watchpoint: bad expression!\n");
      assert(0);  
    }
    else{
      if(ret != p->value){
        printf("Watchpoint %d: %s\n", p->NO, p->expression);
        printf("Old value = %u\n", p->value);
        printf("New value = %u\n", ret);
        
        p->value = ret;
        p->hit = p->hit + 1;
        nemu_state.state = NEMU_STOP;
      }
    }
    p = p->next;
  }
}

void watchpoints_display(){
  if(head == NULL) {
    printf("No watchpoints.\n");
    return;
  }
  WP* p = head;
  printf("Num\tValue\t\tHit\tExpression\n");
  while(p != NULL){
    printf("%d\t",p->NO);
    printf("%-10u\t",p->value);
    printf("%d\t",p->hit);
    printf("%s\n",p->expression);
    p = p->next;
  }
  return;
}

void wp_delete(int num){
  WP* p = head;
  bool found = false;
  while(p != NULL){
    if(p->NO == num){
      found = true;
      free_wp(p);
      break;
    }
  }
  if(found == false)
    printf("No watchpoint number %d\n", num);
  return;
}




