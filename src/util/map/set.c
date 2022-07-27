#include "set.h"

struct __for_each_k_wrapper {
	void (*func)(struct map_entry entry, struct map_for_each_entry *data);
	struct hashset_for_each *key_data;
};

/**
 * @brief wrapper function used to convert hashset for each calls in to map for each calls
 * and allow correct processing
 */
static void __keys_for_each_wrapper(struct map_entry entry,
				    struct map_for_each_entry *data)
{
	struct __for_each_k_wrapper *unwrapped_data =
		(struct __for_each_k_wrapper *)data;

	unwrapped_data->key_data->func(entry.key, unwrapped_data->key_data);
}

void hashset_for_each(hashset_t *set, struct hashset_for_each *data)
{
	struct __for_each_k_wrapper wrapper = (struct __for_each_k_wrapper){
		.func = &__keys_for_each_wrapper,
		.key_data = data,
	};

	map_entries_for_each(set, (struct map_for_each_entry *)&wrapper);
}
