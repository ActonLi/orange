#include "orange_thread.h"
#include "orange_log.h"

void orange_sleep(uint32_t sleep_ms)
{
	return;
}
int orange_thread_create(struct orange_thread_handle* hndl, orange_thread_entry_func entry_func, int pri, void* prm)
{
	return 0;
}
int orange_thread_create_ex(struct orange_thread_handle* hndl, orange_thread_entry_func entry_func, int pri, void* prm, char* p_func, int line)
{
	return 0;
}
int orange_thread_delete(struct orange_thread_handle* hndl)
{
	return 0;
}
int orange_thread_join(struct orange_thread_handle* hndl)
{
	return 0;
}
int orange_thread_join_ex(struct orange_thread_handle* hndl, char* p_func, int line)
{
	return 0;
}
int orange_thread_change_pri(struct orange_thread_handle* hndl, uint32_t pri)
{
	return 0;
}
int orange_thread_exit(void* return_val)
{
	return 0;
}
int orange_thread_is_valid(struct orange_thread_handle* hndl)
{
	return 0;
}
int orange_thread_detach(struct orange_thread_handle* hndl)
{
	return 0;
}
int orange_thread_get_stack_size(void)
{
	return 0;
}
int orange_thread_set_thread_stack_size(uint32_t stack_size)
{
	return 0;
}
int orange_thread_get_self_thread_handle(struct orange_thread_handle* selft_thread_handle)
{
	return 0;
}
int orange_thread_set_thread_pri(struct orange_thread_handle* pthread, int new_pri)
{
	return 0;
}
int orange_thread_get_thread_pri(struct orange_thread_handle* pthread)
{
	return 0;
}
int orange_thread_compare(struct orange_thread_handle* pthread_a, struct orange_thread_handle* pthread_b)
{
	return 0;
}
int orange_thread_parm_create(struct orange_thread_handle* hndl, orange_thread_entry_func thread_func, void* paramter)
{
	return 0;
}
int orange_thread_parm_create_ex(struct orange_thread_handle* hndl, orange_thread_entry_func thread_func, void* paramter, uint32_t pri, int system_scope)
{
	return 0;
}
int orange_thread_cond_event_init(struct orange_cond_event_handle_tag* cond_event)
{
	return 0;
}
int orange_thread_cond_event_de_init(struct orange_cond_event_handle_tag* cond_event)
{
	return 0;
}
int orange_thread_set_cond_event(struct orange_cond_event_handle_tag* cond_event)
{
	return 0;
}
int orange_thread_wait_cond_event(struct orange_cond_event_handle_tag* cond_event, uint32_t timeout_ms)
{
	return 0;
}
int orange_thread_is_cond_event_trig(struct orange_cond_event_handle_tag* cond_event)
{
	return 0;
}
void orange_thread_sleep(uint32_t sleep_ms)
{
	return;
}
