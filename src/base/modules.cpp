#include "modules.hpp"
#include <link.h>
#include <cstring>
#include <cstdio>
namespace trace {
static std::string build_id(const dl_phdr_info* info){
 for(int i=0;i<info->dlpi_phnum;++i){auto& ph=info->dlpi_phdr[i];if(ph.p_type!=PT_NOTE)continue;
 auto p=reinterpret_cast<const unsigned char*>(info->dlpi_addr+ph.p_vaddr);auto end=p+ph.p_memsz;
 while(end-p>=static_cast<int>(sizeof(ElfW(Nhdr)))){
 auto n=reinterpret_cast<const ElfW(Nhdr)*>(p);p+=sizeof(*n);
 auto namesz=(static_cast<std::size_t>(n->n_namesz)+3)&~std::size_t(3),descsz=(static_cast<std::size_t>(n->n_descsz)+3)&~std::size_t(3);
 if(namesz>static_cast<std::size_t>(end-p)||descsz>static_cast<std::size_t>(end-p)-namesz)break;
 if(n->n_type==NT_GNU_BUILD_ID&&n->n_namesz==4&&!std::memcmp(p,"GNU",4)){
 std::string id;char byte[3];for(unsigned j=0;j<n->n_descsz;++j){std::snprintf(byte,3,"%02x",p[namesz+j]);id+=byte;}return id;}
 p+=namesz+descsz;
 }}return {};
}
Module module(const char* name){
 struct Search {const char* name;Module found;} search{name,{}};
 dl_iterate_phdr([](dl_phdr_info* info,std::size_t,void* data){
 auto& s=*static_cast<Search*>(data);const char* path=info->dlpi_name;const char* last=std::strrchr(path,'/');
 if(std::strcmp(last?last+1:path,s.name))return 0;
 s.found={info->dlpi_addr,path,build_id(info)};return 1;
 },&search);return search.found;
}
bool executable(const Module& m,std::uintptr_t address){
 struct Search {std::uintptr_t base,address;bool found;} s{m.base,address,false};
 dl_iterate_phdr([](dl_phdr_info* info,std::size_t,void* data){auto& s=*static_cast<Search*>(data);if(info->dlpi_addr!=s.base)return 0;
 for(int i=0;i<info->dlpi_phnum;++i){auto p=info->dlpi_phdr[i];if(p.p_type==PT_LOAD&&(p.p_flags&PF_X)&&s.address>=s.base+p.p_vaddr&&s.address<s.base+p.p_vaddr+p.p_memsz)s.found=true;}return 1;},&s);
 return s.found;
}
}
