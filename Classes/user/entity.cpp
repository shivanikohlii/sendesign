enum Entity_Type {
  PLAYER_ENTITY = 0, OBSTACLE_ENTITY = 1,
  NUM_ENTITY_TYPES = 2
};
char *entity_type_names[NUM_ENTITY_TYPES] = {
  "Player", "Obstacle"
};

struct Entity {
  int type;
  int texture;
  int manager_index;
  union {
    Rect rect;
    struct {
      union {
	V2 pos;
	struct {float x, y;};
      };
      union {
	V2 dims;
	struct {float w, h;};
      };
    };
  };
  float theta;
  Color4B color;
  int z_order;
  bool invisible;
};

bool save_entity(FILE *file, Entity *e) {
  int characters_written = fprintf(file, "%i %i %f %f %f %f %f %hhu %hhu %hhu %hhu %i %i ",
				   e->type, e->texture, e->x, e->y, e->w, e->h, e->theta,
				   e->color.r, e->color.g, e->color.b, e->color.a,
				   e->z_order, e->invisible ? 1 : 0);
  return characters_written > 0;
}
bool load_entity(FILE *file, Entity *e) {
  int invisible = 0;
  int characters_read = fscanf(file, "%i %i %f %f %f %f %f %hhu %hhu %hhu %hhu %i %i ",
			       &e->type, &e->texture, &e->x, &e->y, &e->w, &e->h, &e->theta,
			       &e->color.r, &e->color.g, &e->color.b, &e->color.a,
			       &e->z_order, &invisible);
  if (characters_read > 0) e->invisible = (invisible != 0);
  return characters_read > 0;
}

Entity *create_entity(Entity *e) {
  Entity *n = new Entity;
  *n = *e;
  if (n->texture < 0 || n->texture >= csp->textures.length) n->texture = 0;

  n->manager_index = csp->entity_manager.length;
  csp->entity_manager.add(n);
  return n;
}
void delete_entity(Entity *e) {
  csp->entity_manager.remove_at(e->manager_index);
  if (e->manager_index < csp->entity_manager.length)
    csp->entity_manager[e->manager_index]->manager_index = e->manager_index;
  delete e;
}

void ordered_delete_entity(Entity *e) { // Call this if you don't want the entity manager to reorder the entities after deleting one
  csp->entity_manager.ordered_remove_at(e->manager_index);
  for (int i = e->manager_index; i < csp->entity_manager.length; i++)
    csp->entity_manager[i]->manager_index = i;
  delete e;
}
inline Entity get_default_entity() {
  Entity e = {};
  e.w = 100.0f;
  e.h = 100.0f;
  e.color = Color4B::WHITE;
  return e;
}

inline bool point_in_entity(V2 point, Entity *e) {
  return point_in_rect(point, e->rect, e->theta*(180.0f/PI));
}
