#ifndef hash_h
#define hash_h

#include "allocator.h"
#include "list.h"

BEGIN_DECLS

#define HASH_IS_REAL(h_) ((h_) >= 2)
#define HASH_FOREACH(_key, _value, hash_table)  \
for (int32_t i = 0; i < hash.capacity(hash_table); i++) \
{                                                       \
    uint32_t node_hash = hash_table->m.hashes[i];       \
    void* _key = hash_table->m.keys[i];                 \
    void* _value = hash_table->m.values[i];

#undef  OBJECT
#define OBJECT hash

/* Object interface */
BASEOBJECT_INTERFACE

int32_t private(size);
int32_t private(mod);
uint32_t private(mask);
int32_t private(nnodes);
int32_t private(noccupied);

void** keys;
uint32_t* hashes;
void** values;

BASEOBJECT_METHODS

ENDOF_INTERFACE

/* Class interface */
CLASS_INTERFACE

t_hash *const classMethod(create);
bool method(destroy);
bool method_(insert)
	void *_key, void *_value __;
bool method_(replace)
	void *_key, void *_value __;
bool method_(add)
	void *_key __;
bool method_(remove)
	void *_key __;
void method(removeAll);
void* method_(lookup)
	const void *_key __;
bool method_(lookupExtended)
	const void *_lookup_key, void **_orig_key, void **_value __;
bool method_(contains)
	const void *_key __;
uint32_t method(capacity);
uint32_t method(size);
t_list* method(getKeys);
t_list* method(getValues);
void** method_(getKeysAsArray)
	uint32_t *_length __;

ENDOF_INTERFACE

END_DECLS

#endif /* hash_h */
