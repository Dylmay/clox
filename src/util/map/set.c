#include "set.h"

struct __for_each_k_wrapper {
	void (*func)(map_entry_t entry, struct __for_each_e *data);
	set_for_each_t *key_data;
};

static void __keys_for_each_wrapper(map_entry_t entry,
				    struct __for_each_e *data)
{
	struct __for_each_k_wrapper *unwrapped_data =
		(struct __for_each_k_wrapper *)data;

	unwrapped_data->key_data->func(entry.key, unwrapped_data->key_data);
}

void set_for_each(hashset_t *set, set_for_each_t *data)
{
	struct __for_each_k_wrapper wrapper = (struct __for_each_k_wrapper){
		.func = &__keys_for_each_wrapper,
		.key_data = data,
	};

	map_entries_for_each(set, (for_each_entry_t *)&wrapper);
}

void *set_find(const hashset_t *set, key_matcher_t *matcher)
{
	return map_find_by_key(set, matcher).value;
}