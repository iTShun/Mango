#define IMPLEMENTATION

#include "hash.h"

#define HASH_TABLE_MIN_SHIFT 3  /* 1 << 3 == 8 buckets */

#define UNUSED_HASH_VALUE 0
#define TOMBSTONE_HASH_VALUE 1
#define HASH_IS_UNUSED(h_) ((h_) == UNUSED_HASH_VALUE)
#define HASH_IS_TOMBSTONE(h_) ((h_) == TOMBSTONE_HASH_VALUE)

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

/* Hash Functions
*/
static bool g_str_equal(const void* v1, const void* v2);

static uint32_t g_str_hash(const void* v);

static void g_hash_table_set_shift(t_hash *hash_table, int32_t shift);

static int32_t g_hash_table_find_closest_shift (int32_t n);

static void g_hash_table_set_shift_from_size(t_hash *hash_table, int32_t size);

static bool g_hash_table_insert_internal(t_hash *hash_table, void* key, void* value, bool keep_new_key);

static bool g_hash_table_insert_node(t_hash *hash_table, uint32_t node_index, uint32_t key_hash, void* new_key, void* new_value, bool keep_new_key, bool reusing_key);

static uint32_t g_hash_table_lookup_node(t_hash *hash_table, const void* key, uint32_t *hash_return);

static void g_hash_table_maybe_resize(t_hash *hash_table);

static void g_hash_table_resize(t_hash *hash_table);

static bool g_hash_table_remove_internal(t_hash *hash_table, const void* key);

static void g_hash_table_remove_node(t_hash *hash_table, int32_t i);

static void g_hash_table_remove_all_nodes(t_hash *hash_table, bool destruction);

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
        this->m.nnodes = 0;
        this->m.noccupied = 0;
		g_hash_table_set_shift(this, HASH_TABLE_MIN_SHIFT);
        
		this->m.keys = BX_ALLOC(getDefaultAllocator(), sizeof(void*) * this->m.size);
		this->m.values = this->m.keys;
		this->m.hashes = BX_ALLOC(getDefaultAllocator(), sizeof(uint32_t) * this->m.size);
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
    return g_hash_table_insert_internal(this, key, key, true);;
}

bool
methodDecl_(remove)
	void *key __
{
    return g_hash_table_remove_internal(this, key);
}

void
methodDecl(removeAll)
{
	assert(this != NULL);

    g_hash_table_remove_all_nodes(this, false);
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

uint32_t
methodDecl(capacity)
{
    assert(this != NULL);
    
    return this->m.size;
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
methodName(capacity),
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

static int32_t g_hash_table_find_closest_shift (int32_t n)
{
    int32_t i;
    
    for (i = 0; n; i++)
        n >>= 1;
    
    return i;
}

static void g_hash_table_set_shift_from_size(t_hash *hash_table, int32_t size)
{
    int32_t shift;
    
    shift = g_hash_table_find_closest_shift (size);
    shift = MAX(shift, HASH_TABLE_MIN_SHIFT);
    
    g_hash_table_set_shift (hash_table, shift);
}

static bool g_hash_table_insert_internal(t_hash *hash_table, void* key, void* value, bool keep_new_key)
{
    uint32_t key_hash;
    uint32_t node_index;
    
    assert (hash_table != NULL);
    
    node_index = g_hash_table_lookup_node(hash_table, key, &key_hash);
    
    return g_hash_table_insert_node(hash_table, node_index, key_hash, key, value, keep_new_key, false);
}

static bool g_hash_table_insert_node(t_hash *hash_table, uint32_t node_index, uint32_t key_hash, void* new_key, void* new_value, bool keep_new_key, bool reusing_key)
{
    bool already_exists;
    uint32_t old_hash;
    
    old_hash = hash_table->m.hashes[node_index];
    already_exists = HASH_IS_REAL (old_hash);
    
    /* Proceed in three steps.  First, deal with the key because it is the
     * most complicated.  Then consider if we need to split the table in
     * two (because writing the value will result in the set invariant
     * becoming broken).  Then deal with the value.
     *
     * There are three cases for the key:
     *
     *  - entry already exists in table, reusing key:
     *    free the just-passed-in new_key and use the existing value
     *
     *  - entry already exists in table, not reusing key:
     *    free the entry in the table, use the new key
     *
     *  - entry not already in table:
     *    use the new key, free nothing
     *
     * We update the hash at the same time...
     */
    if (already_exists)
    {
        if (keep_new_key)
        {
            hash_table->m.keys[node_index] = new_key;
        }
    }
    else
    {
        hash_table->m.hashes[node_index] = key_hash;
        hash_table->m.keys[node_index] = new_key;
    }
    
    /* Step two: check if the value that we are about to write to the
     * table is the same as the key in the same position.  If it's not,
     * split the table.
     */
    if (hash_table->m.keys == hash_table->m.values && hash_table->m.keys[node_index] != new_value)
    {
        uint32_t sz = sizeof (void*) * hash_table->m.size;
        hash_table->m.values = BX_ALLOC(getDefaultAllocator(), sz);
        memCopy(hash_table->m.values, hash_table->m.keys, sz);
    }
    
    /* Step 3: Actually do the write */
    hash_table->m.values[node_index] = new_value;
    
    /* Now, the bookkeeping... */
    if (!already_exists)
    {
        hash_table->m.nnodes++;
        
        if (HASH_IS_UNUSED (old_hash))
        {
            /* We replaced an empty node, and not a tombstone */
            hash_table->m.noccupied++;
            g_hash_table_maybe_resize(hash_table);
        }
    }
    
    return !already_exists;
}

static inline uint32_t g_hash_table_lookup_node(t_hash *hash_table, const void* key, uint32_t *hash_return)
{
    uint32_t node_index;
    uint32_t node_hash;
    uint32_t hash_value;
    uint32_t first_tombstone = 0;
    bool have_tombstone = false;
    uint32_t step = 0;
    
    hash_value = g_str_hash (key);
    if (!HASH_IS_REAL (hash_value))
        hash_value = 2;
    
    *hash_return = hash_value;
    
    node_index = hash_value % hash_table->m.mod;
    node_hash = hash_table->m.hashes[node_index];
    
    while (!HASH_IS_UNUSED (node_hash))
    {
        /* We first check if our full hash values
         * are equal so we can avoid calling the full-blown
         * key equality function in most cases.
         */
        if (node_hash == hash_value)
        {
            void* node_key = hash_table->m.keys[node_index];
            
            if (g_str_equal(node_key, key))
                return node_index;
        }
        else if (HASH_IS_TOMBSTONE (node_hash) && !have_tombstone)
        {
            first_tombstone = node_index;
            have_tombstone = true;
        }
        
        step++;
        node_index += step;
        node_index &= hash_table->m.mask;
        node_hash = hash_table->m.hashes[node_index];
    }
    
    if (have_tombstone)
        return first_tombstone;
    
    return node_index;
}

static inline void g_hash_table_maybe_resize(t_hash *hash_table)
{
    int32_t noccupied = hash_table->m.noccupied;
    int32_t size = hash_table->m.size;
    
    if ((size > hash_table->m.nnodes * 4 && size > 1 << HASH_TABLE_MIN_SHIFT) ||
        (size <= noccupied + (noccupied / 16)))
        g_hash_table_resize(hash_table);
}

static void g_hash_table_resize(t_hash *hash_table)
{
    void** new_keys;
    void** new_values;
    uint32_t *new_hashes;
    int32_t old_size;
    int32_t i;
    
    old_size = hash_table->m.size;
    g_hash_table_set_shift_from_size(hash_table, hash_table->m.nnodes * 2);
    
    new_keys = BX_ALLOC(getDefaultAllocator(), sizeof(void*) * hash_table->m.size);
    if (hash_table->m.keys == hash_table->m.values)
        new_values = new_keys;
    else
        new_values = BX_ALLOC(getDefaultAllocator(), sizeof(void*) * hash_table->m.size);
    new_hashes = BX_ALLOC(getDefaultAllocator(), sizeof(uint32_t) * hash_table->m.size);
    
    for (i = 0; i < old_size; i++)
    {
        uint32_t node_hash = hash_table->m.hashes[i];
        uint32_t hash_val;
        uint32_t step = 0;
        
        if (!HASH_IS_REAL(node_hash))
            continue;
        
        hash_val = node_hash % hash_table->m.mod;
        
        while (!HASH_IS_UNUSED (new_hashes[hash_val]))
        {
            step++;
            hash_val += step;
            hash_val &= hash_table->m.mask;
        }
        
        new_hashes[hash_val] = hash_table->m.hashes[i];
        new_keys[hash_val] = hash_table->m.keys[i];
        new_values[hash_val] = hash_table->m.values[i];
    }
    
    if (hash_table->m.keys != hash_table->m.values)
        BX_SAFEFREE(getDefaultAllocator(), hash_table->m.values);
    
    BX_SAFEFREE(getDefaultAllocator(), hash_table->m.keys);
    BX_SAFEFREE(getDefaultAllocator(), hash_table->m.hashes);
    
    hash_table->m.keys = new_keys;
    hash_table->m.values = new_values;
    hash_table->m.hashes = new_hashes;
    
    hash_table->m.noccupied = hash_table->m.nnodes;
}

static bool g_hash_table_remove_internal(t_hash *hash_table, const void* key)
{
    uint32_t node_index;
    uint32_t node_hash;
    
    assert(hash_table != NULL);
    
    node_index = g_hash_table_lookup_node(hash_table, key, &node_hash);
    
    if (!HASH_IS_REAL(hash_table->m.hashes[node_index]))
        return false;
    
    g_hash_table_remove_node(hash_table, node_index);
    g_hash_table_maybe_resize(hash_table);
    
    return true;
}

static void g_hash_table_remove_node(t_hash *hash_table, int32_t i)
{
    void* key;
    void* value;
    
    key = hash_table->m.keys[i];
    value = hash_table->m.values[i];
    
    /* Erect tombstone */
    hash_table->m.hashes[i] = TOMBSTONE_HASH_VALUE;
    
    /* Be GC friendly */
    hash_table->m.keys[i] = NULL;
    hash_table->m.values[i] = NULL;
    
    hash_table->m.nnodes--;
}

static void g_hash_table_remove_all_nodes(t_hash *hash_table, bool destruction)
{
    int32_t i;
    void* key;
    void* value;
    int32_t old_size;
    void** old_keys;
    void** old_values;
    uint32_t *old_hashes;
    
    key = value = old_keys = old_values = old_hashes = NULL;
    
    /* If the hash table is already empty, there is nothing to be done. */
    if (hash_table->m.nnodes == 0)
        return;
    
    hash_table->m.nnodes = 0;
    hash_table->m.noccupied = 0;
    
    /* Keep the old storage space around to iterate over it. */
    old_size = hash_table->m.size;
    old_keys   = hash_table->m.keys;
    old_values = hash_table->m.values;
    old_hashes = hash_table->m.hashes;
    
    /* Now create a new storage space; If the table is destroyed we can use the
     * shortcut of not creating a new storage. This saves the allocation at the
     * cost of not allowing any recursive access.
     * However, the application doesn't own any reference anymore, so access
     * is not allowed. If accesses are done, then either an assert or crash
     * *will* happen. */
    g_hash_table_set_shift(hash_table, HASH_TABLE_MIN_SHIFT);
    if (!destruction)
    {
        hash_table->m.keys   = BX_ALLOC(getDefaultAllocator(), sizeof(void*) * hash_table->m.size);
        hash_table->m.values = hash_table->m.keys;
        hash_table->m.hashes = BX_ALLOC(getDefaultAllocator(), sizeof(uint32_t) * hash_table->m.size);
    }
    else
    {
        hash_table->m.keys   = NULL;
        hash_table->m.values = NULL;
        hash_table->m.hashes = NULL;
    }
    
    /* Destroy old storage space. */
    if (old_keys != old_values)
        BX_SAFEFREE(getDefaultAllocator(), old_values);
    
    BX_SAFEFREE(getDefaultAllocator(), old_keys);
    BX_SAFEFREE(getDefaultAllocator(), old_hashes);
}
