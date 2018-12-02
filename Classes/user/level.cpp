#ifdef _WIN32
// Windows Version:
inline bool file_exists(char *file) {
  DWORD attrib = GetFileAttributesA(file);
  return (attrib != INVALID_FILE_ATTRIBUTES &&  !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}
#else
// Linux Version:
inline bool file_exists(char *file) {
  return (stat(pathname, &sb) == 0 && S_ISREG(sb.st_mode));
}
#endif

#define MAX_LEVEL_PATH_SIZE 256

inline bool level_exists(char *level_name) {
  char full_level_path[MAX_LEVEL_PATH_SIZE]; //@MEMORY
  snprintf(full_level_path, sizeof(full_level_path), "data/levels/%s.level", level_name);
  return file_exists(full_level_path);
}

char current_level_name[MAX_LEVEL_PATH_SIZE] = {};

bool save_level(char *level_name) {
  char full_level_path[MAX_LEVEL_PATH_SIZE]; //@MEMORY
  snprintf(full_level_path, sizeof(full_level_path), "data/levels/%s.level", level_name);
  FILE *file = fopen(full_level_path, "w"); //@CHECK
  if (!file) return false;
  
  int error = fprintf(file, "%i ", csp->entity_manager.length);
  if (error < 0) {
    fclose(file);
    return false;
  }
  for (int i = 0; i < csp->entity_manager.length; i++) {
    Entity *e = csp->entity_manager[i];
    bool success = save_entity(file, e);
    if (!success) {
      fclose(file);
      return false;
    }
  }
  fclose(file);
  
  return true;
}

bool load_level(char *level_name) {
  char full_level_path[MAX_LEVEL_PATH_SIZE]; //@MEMORY
  snprintf(full_level_path, sizeof(full_level_path), "data/levels/%s.level", level_name);
  FILE *file = fopen(full_level_path, "r");
  if (!file) return false;
  strncpy(current_level_name, level_name, sizeof(current_level_name));
  while (csp->entity_manager.length > 0) delete_entity(csp->entity_manager[0]);
  
  int num_entities = 0;
  int error = fscanf(file, "%i ", &num_entities);
  if (error < 0) {
    fclose(file);
    return false;
  }
  for (int i = 0; i < num_entities; i++) {
    Entity e = {};
    int invisible = 0;
    bool success = load_entity(file, &e);
    if (!success) {
      fclose(file);
      return false;
    }
    create_entity(&e);
  }
  fclose(file);

#ifdef _DEBUG
  editor.selected_entity = NULL;
#endif

  return true;
}
