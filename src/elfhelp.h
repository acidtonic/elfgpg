#ifndef ELFSIGN_ELFHELP_H
#define ELFSIGN_ELFHELP_H

/* create_data() and create_section() was taken from libelf-examples-0.1.0 
* written by Michael Riepe <michael@stud.uni-hannover.de>
*/

#define elferr(str) fprintf(stderr, "%s: %s\n", str, elf_errmsg(-1))

static Elf_Data*
create_data(Elf_Scn *scn, void *buf, size_t size, Elf_Type type, size_t align) {
    Elf_Data *data;

    if ((data = elf_newdata(scn))) {
	data->d_align = align;
	data->d_size = size;
	data->d_type = type;
	data->d_buf = buf;
	return data;
    }
    elferr("newdata");
    return NULL;
}

static Elf_Scn*
create_section(Elf *elf, unsigned type, char *name) {
    static char shstrtab[] = ".shstrtab";
    Elf32_Ehdr *ehdr;
    Elf32_Shdr *shdr;
    Elf_Data *data;
    Elf_Scn *scn, *strscn;

    if (!(ehdr = elf32_getehdr(elf))) {
	elferr("ELF header");
	return NULL;
    }
    if (!ehdr->e_shstrndx) {
	ehdr->e_version = EV_CURRENT;
	if (!(shdr = elf32_getshdr(strscn = elf_newscn(elf)))) {
	    elferr("section name table");
	    return NULL;
	}
	shdr->sh_type = SHT_STRTAB;
	ehdr->e_shstrndx = elf_ndxscn(strscn);
	if (!create_data(strscn, "", 1, ELF_T_BYTE, 1)) {
	    return NULL;
	}
	if (!(data = create_data(strscn, shstrtab, sizeof(shstrtab), ELF_T_BYTE, 1))) {
	    return NULL;
	}
	if (elf_update(elf, ELF_C_NULL) == -1) {
	    elferr("update");
	    return NULL;
	}
	shdr->sh_name = data->d_off;
    }
    else if (!(strscn = elf_getscn(elf, ehdr->e_shstrndx))) {
	elferr("section name table");
	return NULL;
    }
    if (!(shdr = elf32_getshdr(scn = elf_newscn(elf)))) {
	elferr("newscn");
	return NULL;
    }
    if (!(data = create_data(strscn, name, 1 + strlen(name), ELF_T_BYTE, 1))) {
	return NULL;
    }
    if (elf_update(elf, ELF_C_NULL) == -1) {
	elferr("update");
	return NULL;
    }
    shdr->sh_name = data->d_off;
    shdr->sh_type = type;
    return scn;
}

static Elf_Scn*
elf_findscn(Elf *elf, const char *name) {
    Elf32_Ehdr *ehdr;
    Elf32_Shdr *shdr;
    Elf_Scn *scn;
    char *str;

    if ((ehdr = elf32_getehdr(elf))) {
	scn = NULL;
	while ((shdr = elf32_getshdr(scn = elf_nextscn(elf, scn)))) {
	    if (shdr->sh_type == SHT_NULL) {
		continue;
	    }
	    if (!(str = elf_strptr(elf, ehdr->e_shstrndx, shdr->sh_name))) {
		break;
	    }
	    if (!strcmp(str, name)) {
		return scn;
	    }
	}
    }
    return NULL;
}


#endif