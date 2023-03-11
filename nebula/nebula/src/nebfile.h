#ifndef NEBFILE_H
#define NEBFILE_H

void nebfile_init(void);

int nebfile_close(void);
struct fileinfo *nebfile_open(const char *filename);
void nebfile_save(struct fileinfo *finfo);
int nebfile_open_data_file(const char *filename,struct tags *parsers);
#endif /* NEBFILE_H */
