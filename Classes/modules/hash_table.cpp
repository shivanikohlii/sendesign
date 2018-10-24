#define DEFAULT_HASH_TABLE_SIZE 128

u64 djb2_hash(unsigned char *str) {
  u64 hash = 5381;
  int c;
  while (c = *str++) hash = ((hash << 5) + hash) + c;
  return hash;
}

template <class Key_Type, class Data_Type> struct Hash_Record {
  Key_Type key;
  Data_Type data;
};
template <class Key_Type, class Data_Type> struct Hash_Slot {
  Dynamic_Array<Hash_Record<Key_Type, Data_Type>> *entries;
};
template <class Key_Type, class Data_Type> struct Hash_Table {
  Dynamic_Array<Hash_Slot<Key_Type, Data_Type>> slots;
  u64 (*hash_function)(Key_Type);
  int (*key_compare)(Key_Type, Key_Type);

  Hash_Table(u64 (*in_hash_function)(Key_Type), int (*in_key_compare)(Key_Type, Key_Type)) {
	  hash_function = in_hash_function;
	  key_compare = in_key_compare;
  }

  inline u64 get_hash_table_slot_index(u64 key) {
    return key % slots.length;
  }

  bool remove(Key_Type key) {
    if (slots.length == 0) return false;
    u64 slot_index = hash_function(key) % slots.length;
    Hash_Slot<Key_Type, Data_Type> slot = slots[slot_index];
    if (!slot.entries) return false;
    for (int i = 0; i < slot.entries->length; i++) {
      if (key_compare(slot.entries->data[i].key, key) == 0) {
	slot.entries->remove_at(i);
	return true;
      }
    }
  }
  void add_to_hash_table(Key_Type key, Data_Type data) {
    if (slots.length == 0) {
      slots.set_length(DEFAULT_HASH_TABLE_SIZE);
      memset(slots.data, 0, sizeof(Hash_Slot<Key_Type, Data_Type>)*DEFAULT_HASH_TABLE_SIZE);
    }
    u64 slot_index = hash_function(key) % slots.length; //get_hash_table_slot_index(key);
    Hash_Record<Key_Type, Data_Type> record;
    record.key = key;
    record.data = data;
    Hash_Slot<Key_Type, Data_Type> b = slots[slot_index];
    if (!b.entries) {
      b.entries = (Dynamic_Array<Hash_Record<Key_Type, Data_Type>> *)malloc(sizeof(Dynamic_Array<Hash_Record<Key_Type, Data_Type>>));
      b.entries->initialize(4); // 4 item slots to start
      slots[slot_index] = b;
    }
    slots[slot_index].entries->add(record);
  }
  /*inline void add_to_hash_table(Data_Type data, char *key) {
    add_to_hash_table(data, djb2_hash((unsigned char *)key));
    }*/

  bool retrieve_from_hash_table(Key_Type key, Data_Type *out) {
    if (slots.length == 0) return 0;
    u64 slot_index = hash_function(key) % slots.length;
    Hash_Slot<Key_Type, Data_Type> slot = slots[slot_index];
    if (slot.entries) {
      for (int i = 0; i < slot.entries->length; i++) {
	if (key_compare(slot.entries->data[i].key, key) == 0) {
	  *out = slot.entries->data[i].data;
	  return true;
	}
      }
    }
    return false;
  }

  inline Dynamic_Array<Hash_Record<Key_Type, Data_Type>> *retrieve_records_from_hash_table(u64 key) {
    if (slots.length == 0) return NULL;
    return slots[get_hash_table_slot_index(key)].entries;
  }
};


