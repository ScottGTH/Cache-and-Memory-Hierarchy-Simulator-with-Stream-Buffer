#ifndef cache_block_block_H
#define cache_block_block_H

#define address_bits 32

#include <cstdio>
#include <iostream>
#include <cstdlib>
using namespace std;

typedef struct {
    uint32_t tag;
    bool is_valid;
    bool is_dirty;
    bool unit_is_valid;
    unsigned int lru_order;
    uint32_t unit_lru_order;
    uint32_t addr;

}Block;

class Cache{
  public:

    uint32_t Cache_size;
    uint32_t Block_size;
    uint32_t Associativity;
    uint32_t Pref_Unit;
    uint32_t Pref_Size;
    
    unsigned int cache_level;
    bool is_asso;
    bool N_last_level;
    bool Pref_Exist;
    
    uint32_t hit_pref_unit;
    uint32_t hit_pref_block;
    uint32_t hit_way_pos;
    
    uint32_t block_offset_bits;
    uint32_t sets;
    uint32_t index_bits;
    uint32_t tag_bits;

    uint32_t rd, wr, rd_miss, wr_miss, wr_back, mem_traffic;
    uint32_t prefetch, rd_with_pref, wr_with_pref, read_miss_wtih_pref;
    
    uint32_t index_extract;
    
    
    
    uint32_t first_lru_unit_pos;
    uint32_t last_lru_unit_pos;
    uint32_t v_block_way;
    uint32_t v_block_addr;
    uint32_t v_addr;
    int Next_L_Behavior;
    int Cache_level;
    
    Block** CacheBlock;
    Block** VC_Cache;
    Block V_BLock;

    Cache* Next_L_Ca;
    
    Cache(
        uint32_t Csize,
        uint32_t Bsize,
        uint32_t CBasso,
        uint32_t PREF_N,
        uint32_t PREF_M,
        bool N_L_V,
        unsigned int level_name
    );

    virtual ~Cache() {delete CacheBlock;}


    

    uint32_t Get_Tag(uint32_t addr);

    uint32_t Get_Set(uint32_t addr);

    uint32_t Get_Offset(uint32_t addr);

    bool Block_Hit(uint32_t set_pos, uint32_t tag_content);

    void Get_V_Block_Way(uint32_t set_pos, bool is_asso);

    void Get_VC_Unit_Way();

    void Get_1st_LRU_Unit();

    void Swap(uint32_t swap_start_pos);

    bool exist_vc(uint32_t tag_content, uint32_t lru_pos, uint32_t addr);

    void pref_miss(uint32_t addr, uint32_t unit_pos);
    
    void pref_contents_upd(uint32_t addr);  //uint32_t hit_pref_unit, uint32_t hit_pref_block

    void vcblock_lru_upd(uint32_t unit_pos);
    
    void vc_to_c_transfer(uint32_t set_pos, uint32_t way_pos);

    void WA(uint32_t set_pos, uint32_t tag_content, uint32_t way_pos, bool need_wa, bool set_dirty);

    void rd_req(uint32_t addr);

    void wr_req(uint32_t addr);

    void req(char rw, uint32_t addr);

    void lru_update(uint32_t set_pos, uint32_t way_pos);

    void NL_Rdreq(char rw, uint32_t addr);
    void NL_Wrreq(char rw, uint32_t v_block_addr);
    void contents();

    //void print_measu();

    /*uint32_t get_V_Block_Way(uint32_t set_pos);
    Block* get_victim_block (uint32_t set_pos);
    Block* get_victim_block ();*/
};

#endif