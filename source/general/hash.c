#define IMPLEMENTATION

#include "hash.h"

#define HASH_TABLE_MIN_SHIFT 3  /* 1 << 3 == 8 buckets */

#define UNUSED_HASH_VALUE 0
#define TOMBSTONE_HASH_VALUE 1
#define HASH_IS_UNUSED(h_) ((h_) == UNUSED_HASH_VALUE)
#define HASH_IS_TOMBSTONE(h_) ((h_) == TOMBSTONE_HASH_VALUE)
#define HASH_IS_REAL(h_) ((h_) >= 2)

/* Each table size has an associated prime modulo (the first prime
* lower than the table size) used to find the initial bucket. Probing
* then works modulo 2^n. The prime modulo is necessary to get a
* good distribution with poor hash functions.
*/
static const int32_t prime_mod[] =
{
	1,          /* For 1 << 0 */
	2,
	3,
	7,
	13,
	31,
	61,
	127,
	251,
	509,
	1021,
	2039,
	4093,
	8191,
	16381,
	32749,
	65521,      /* For 1 << 16 */
	131071,
	262139,
	524287,
	1048573,
	2097143,
	4194301,
	8388593,
	16777213,
	33554393,
	67108859,
	134217689,
	268435399,
	536870909,
	1073741789,
	2147483647  /* For 1 << 31 */
};

typedef struct
{
	t_hash		*hash_table;
	void*		dummy1;
	void*		dummy2;
	int32_t     position;
	bool		dummy3;
	int32_t     dummy4;
} RealIter;

/* Hash Functions
*/
static bool g_str_equal(const void* v1, const void* v2);

static uint32_t g_str_hash(const void* v);

static void g_hash_table_set_shift(t_hash *hash_table, int32_t shift);

/*
 ---------------------
 Object implementation
 ---------------------
 */

BASEOBJECT_IMPLEMENTATION

ENDOF_IMPLEMENTATION

/*
 --------------------
 Class implementation
 --------------------
 */

initClassDecl() /* required */
{
    
}

dtorDecl() /* required */
{
	hash.destroy(this);
}

t_hash
classMethodDecl(*const create)
{
	t_hash *const this = hash.alloc();
	if (this)
	{
		memSet(this, 0, sizeof(t_hash));

		g_hash_table_set_shift(this, HASH_TABLE_MIN_SHIFT);

		this->m.keys = BX_ALLOC(getDefaultAllocator(), sizeof(void*) * this->m.size);
		this->m.values = this->m.keys;
		this->m.hashes = BX_ALLOC(getDefaultAllocator(), sizeof(uint32_t) * this->m.size);

		memSet(this->m.keys, 0, sizeof(void*) * this->m.size);
		memSet(this->m.hashes, 0, sizeof(uint32_t) * this->m.size);
	}
	else
		delete(this);

	return this;
}

bool
methodDecl(destroy)
{
	assert(this != NULL);

	if (this->m.keys != this->m.values)
		BX_SAFEFREE(getDefaultAllocator(), this->m.keys);

	BX_SAFEFREE(getDefaultAllocator(), this->m.keys);
	BX_SAFEFREE(getDefaultAllocator(), this->m.hashes);

	return true;
}

bool 
methodDecl_(insert)
	void *key, void *value __
{
	return g_hash_table_insert_internal(this, key, value, false);
}

bool
methodDecl_(replace)
	void *key, void *value __
{
	return g_hash_table_insert_internal(this, key, value, true);
}

bool 
methodDecl_(add)
	void *key __
{
	return g_hash_table_insert_internal(this, key, key, true);
}

bool
methodDecl_(remove)
	void *key __
{
	return g_hash_table_remove_internal(this, key, true);
}

bool 
methodDecl(removeAll)
{
	assert(this != NULL);

	g_hash_table_remove_all_nodes(this, true, false);
	g_hash_table_maybe_resize(this);

	return true;
}

bool
methodDecl_(steal)
	void *key __
{
	return g_hash_table_remove_internal(this, key, false);
}

bool 
methodDecl_(stealExtended)
	const void *lookup_key, void **stolen_key, void **stolen_value __
{
	uint32_t node_index;
	uint32_t node_hash;

	assert(this != NULL);

	node_index = g_hash_table_lookup_node(this, lookup_key, &node_hash);

	if (!HASH_IS_REAL(this->m.hashes[node_index]))
	{
		if (stolen_key != NULL)
			*stolen_key = NULL;
		if (stolen_value != NULL)
			*stolen_value = NULL;
		return false;
	}

	if (stolen_key != NULL)
		*stolen_key = &this->m.keys[node_index];

	if (stolen_value != NULL)
		*stolen_value = &this->m.values[node_index];

	g_hash_table_remove_node(this, node_index, false);
	g_hash_table_maybe_resize(this);

	return true;
}

void 
methodDecl(stealAll)
{
	assert(this != NULL);

	g_hash_table_remove_all_nodes(this, false, false);
	g_hash_table_maybe_resize(this);
}

void
methodDecl_(*const lookup)
	const void *key __
{
	uint32_t node_index;
	uint32_t node_hash;

	assert(this != NULL);

	node_index = g_hash_table_lookup_node(this, key, &node_hash);

	return HASH_IS_REAL(this->m.hashes[node_index])
		? this->m.values[node_index]
		: NULL;
}

bool 
methodDecl_(lookupExtended)
	const void *lookup_key, void **orig_key, void **value __
{
	uint32_t node_index;
	uint32_t node_hash;

	assert(this != NULL);

	node_index = g_hash_table_lookup_node(this, lookup_key, &node_hash);

	if (!HASH_IS_REAL(this->m.hashes[node_index]))
		return false;

	if (orig_key)
		*orig_key = this->m.keys[node_index];

	if (value)
		*value = this->m.values[node_index];

	return true;
}

bool 
methodDecl_(contains)
	const void *key __
{
	uint32_t node_index;
	uint32_t node_hash;

	assert(this != NULL);

	node_index = g_hash_table_lookup_node(this, key, &node_hash);

	return HASH_IS_REAL(this->m.hashes[node_index]);
}

void
methodDecl_(*const find)
	HRFunc predicate, void *user_data __
{
	int32_t i;
	bool match;

	assert(this != NULL);
	assert(predicate != NULL);

	match = false;

	for (i = 0; i < this->m.size; i++)
	{
		uint32_t node_hash = this->m.hashes[i];
		void* node_key = this->m.keys[i];
		void* node_value = this->m.values[i];

		if (HASH_IS_REAL(node_hash))
			match = predicate(node_key, node_value, user_data);

		if (match)
			return node_value;
	}

	return NULL;
}

uint32_t 
methodDecl(size)
{
	assert(this != NULL);

	return this->m.nnodes;
}

t_list 
methodDecl(*const getKeys)
{
	assert(this != NULL);

	int32_t i;
	t_list* retval = list.create();

	for (i = 0; i < this->m.size; i++)
	{
		if (HASH_IS_REAL(this->m.hashes[i]))
			list.add(retval, this->m.keys[i]);
	}

	return retval;
}

t_list 
methodDecl(*const getValues)
{
	assert(this != NULL);

	int32_t i;
	t_list* retval = list.create();

	for (i = 0; i < this->m.size; i++)
	{
		if (HASH_IS_REAL(this->m.hashes[i]))
			list.add(retval, this->m.values[i]);
	}

	return retval;
}

void
methodDecl_(**const getKeysAsArray)
	uint32_t *length __
{
	assert(this != NULL);

	void** result = NULL;
	uint32_t i, j = 0;

	result = BX_ALLOC(getDefaultAllocator(), sizeof(void*) * (this->m.nnodes + 1));
	for (i = 0; i < this->m.size; i++)
	{
		if (HASH_IS_REAL(this->m.hashes[i]))
			result[j++] = this->m.keys[i];
	}
	if (j != this->m.nnodes)
	{
		BX_SAFEFREE(getDefaultAllocator(), result);
		return NULL;
	}
	result[j] = NULL;

	if (length)
		*length = j;

	return result;
}

CLASS_IMPLEMENTATION

methodName(create),
methodName(destroy),
methodName(insert),
methodName(replace),
methodName(add),
methodName(remove),
methodName(removeAll),
methodName(lookup),
methodName(lookupExtended),
methodName(contains),
methodName(size),
methodName(getKeys),
methodName(getValues),
methodName(getKeysAsArray)

ENDOF_IMPLEMENTATION


static bool g_str_equal(const void* v1, const void* v2)
{
	const char *string1 = v1;
	const char *string2 = v2;

	return strcmp(string1, string2) == 0;
}

static uint32_t g_str_hash(const void* v)
{
	const signed char *p;
	uint32_t h = 5381;

	for (p = v; *p != '\0'; p++)
		h = (h << 5) + h + *p;

	return h;
}

static void g_hash_table_set_shift(t_hash *hash_table, int32_t shift)
{
	int32_t i;
	uint32_t mask = 0;

	hash_table->m.size = 1 << shift;
	hash_table->m.mod = prime_mod[shift];

	for (i = 0; i < shift; i++)
	{
		mask <<= 1;
		mask |= 1;
	}

	hash_table->m.mask = mask;
}
