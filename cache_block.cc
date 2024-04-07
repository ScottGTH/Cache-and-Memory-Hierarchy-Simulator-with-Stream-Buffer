#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <inttypes.h>
#include <tuple>
#include <list>
#include <vector>
#include <bitset>
#include <string>
#include "cache_block.h"
using namespace std;

Cache::Cache(uint32_t Csize, uint32_t Bsize, uint32_t CBasso,
uint32_t PREF_N, uint32_t PREF_M, bool N_L_V, unsigned int level_name){
    Cache_size = Csize;
    Block_size = Bsize;
    Associativity = CBasso;
    Pref_Unit = PREF_N;
    Pref_Size = PREF_M;

    
    cache_level = level_name;

    is_asso = (CBasso > 1);
    N_last_level = N_L_V;

    Pref_Exist = (PREF_N);

    hit_pref_unit = 0;
    hit_pref_block = 0;
    hit_way_pos = 0;

    block_offset_bits = log2(Block_size);
    sets = Cache_size / (Block_size * Associativity);
    index_bits = log2(sets);
    tag_bits = address_bits - index_bits - block_offset_bits;

    rd = wr = rd_miss = wr_miss = wr_back = mem_traffic = 0; 
    prefetch = 0;
    rd_with_pref = wr_with_pref = read_miss_wtih_pref = 0;
    
    

    index_extract = 0;
    for(unsigned int i=0; i < index_bits; i++) {
        index_extract <<= 1;
        index_extract |= 1;

    }

    CacheBlock = new Block* [sets];	
    for(unsigned int i=0; i < sets; i++) {
        CacheBlock[i] = new Block[Associativity];	
        for(unsigned int j=0; j < Associativity; j++) {
            CacheBlock[i][j].tag = 0;
            CacheBlock[i][j].is_valid = false;
            CacheBlock[i][j].is_dirty = false;
            CacheBlock[i][j].lru_order = j;
        }
    }

    
    VC_Cache = new Block* [Pref_Unit];
    for(unsigned int i=0; i < Pref_Unit+1; i++) {
        VC_Cache[i] = new Block[Pref_Size];	
        //VC_Cache[i]->is_valid = false;   // new try, need focus. try, need focus. using to tell wether this stream buffer is used
        //VC_Cache[i]->lru_order = i; // new try, need focus. try, need focus. using to set multi stream buffer LRU
        for(unsigned int j=0; j < Pref_Size; j++) {
            VC_Cache[i][j].tag = 0;
            VC_Cache[i][j].addr = 0;
            VC_Cache[i][j].is_valid = false;
            VC_Cache[i][j].lru_order = j;
            VC_Cache[i][j].unit_lru_order = i;
            VC_Cache[i][j].unit_is_valid = false;
        }
    }
    
}


bool Cache::Block_Hit(uint32_t set_pos, uint32_t tag_content){
    for(unsigned int j=0; j < Associativity; j++){
        if(CacheBlock[set_pos][j].is_valid){
            if(CacheBlock[set_pos][j].tag == tag_content){
                hit_way_pos = j;
                return true;
            } 
        }
    }
    return false;
}

void Cache::Get_V_Block_Way(uint32_t set_pos, bool is_asso){
    v_block_way = 0;
    if(is_asso){
        for(unsigned int i=1; i < Associativity; i++){
            if(CacheBlock[set_pos][i].lru_order > CacheBlock[set_pos][v_block_way].lru_order){
                v_block_way = i;
            }
        }
    }
}

void Cache::Get_VC_Unit_Way(){
    last_lru_unit_pos = 0;
    for(unsigned int i=1; i < Pref_Unit; i++){
        if(VC_Cache[i][0].unit_lru_order > VC_Cache[last_lru_unit_pos][0].unit_lru_order){
            last_lru_unit_pos = i;
        }
    }
}

void Cache::Get_1st_LRU_Unit(){
    first_lru_unit_pos = 0;
    for(unsigned int i=1; i < Pref_Unit; i++){
        if(VC_Cache[i][0].unit_lru_order < VC_Cache[first_lru_unit_pos][0].unit_lru_order){
            first_lru_unit_pos = i;
        }
    }
}

void Cache::Swap(uint32_t swap_start_pos){
    for(unsigned int j=0; j < Pref_Size; j++){
        VC_Cache[Pref_Unit][j].addr =  VC_Cache[swap_start_pos][j].addr;
        VC_Cache[Pref_Unit][j].tag = VC_Cache[Pref_Unit][j].addr >> index_bits;
        VC_Cache[Pref_Unit][j].unit_is_valid = VC_Cache[swap_start_pos][j].unit_is_valid;
    }

    for(unsigned int i= (swap_start_pos); i > 0; i--){
        for(unsigned int j=0; j < Pref_Size; j++){
            VC_Cache[i][j].addr =  VC_Cache[(i-1)][j].addr;
            VC_Cache[i][j].tag = VC_Cache[i][j].addr >> index_bits;
            VC_Cache[i][j].unit_is_valid = VC_Cache[(i-1)][j].unit_is_valid;
        }        
    }
    for(unsigned int j=0; j < Pref_Size; j++){
        VC_Cache[0][j].addr =  VC_Cache[Pref_Unit][j].addr;
        VC_Cache[0][j].tag = VC_Cache[0][j].addr >> index_bits;
        VC_Cache[0][j].unit_is_valid = VC_Cache[Pref_Unit][j].unit_is_valid;
    }
}

bool Cache::exist_vc(uint32_t tag_content, uint32_t lru_pos, uint32_t addr){
    if(Pref_Unit > 1){
        /*int lru_state = 0;
        unsigned int unit_lru = 0;
        unsigned int block_lru = 0;
        if(VC_Cache[0][0].unit_lru_order == 2){
            if(VC_Cache[0][0].unit_is_valid){
                for(unsigned int j=0; j < Pref_Size; j++){
                    if(VC_Cache[0][j].tag == tag_content){
                            unit_lru = 0;
                            block_lru = j;        
                    }
                    lru_state = 0;
                }
                if(VC_Cache[1][0].unit_lru_order > VC_Cache[2][0].unit_lru_order){
                    for(unsigned int j=0; j < Pref_Size; j++){
                        if(VC_Cache[1][j].tag == tag_content){
                                unit_lru = 1;
                                block_lru = j;
                        }
                        lru_state = 1;
                    }
                }else{
                    for(unsigned int j=0; j < Pref_Size; j++){
                        if(VC_Cache[2][j].tag == tag_content){
                                unit_lru = 2;
                                block_lru = j;
                        }
                        lru_state = 2;
                    }   
                }
            }else{
                return false;
            }
        }else if(VC_Cache[1][0].unit_lru_order == 2){
            if(VC_Cache[1][0].unit_is_valid){
                for(unsigned int j=0; j < Pref_Size; j++){
                    if(VC_Cache[1][j].tag == tag_content){
                            unit_lru = 1;
                            block_lru = j;
                    }
                    lru_state = 1;
                }
                if(VC_Cache[0][0].unit_lru_order > VC_Cache[2][0].unit_lru_order){
                    for(unsigned int j=0; j < Pref_Size; j++){
                        if(VC_Cache[0][j].tag == tag_content){
                                unit_lru = 0;
                                block_lru = j;
                        }
                        lru_state = 0;
                    }
                }else{
                    for(unsigned int j=0; j < Pref_Size; j++){
                        if(VC_Cache[2][j].tag == tag_content){
                                unit_lru = 2;
                                block_lru = j;
                        }
                        lru_state = 2;
                    }   
                }
            }else{
                return false;
            }
        }else if(VC_Cache[2][0].unit_lru_order == 2){
            if(VC_Cache[2][0].unit_is_valid){
                for(unsigned int j=0; j < Pref_Size; j++){
                    if(VC_Cache[2][j].tag == tag_content){
                            unit_lru = 2;
                            block_lru = j;
                    }
                    lru_state = 2;
                }
                if(VC_Cache[0][0].unit_lru_order > VC_Cache[1][0].unit_lru_order){
                    for(unsigned int j=0; j < Pref_Size; j++){
                        if(VC_Cache[0][j].tag == tag_content){
                                unit_lru = 0;
                                block_lru = j;
                        }
                        lru_state = 0;
                    }
                }else{
                    for(unsigned int j=0; j < Pref_Size; j++){
                        if(VC_Cache[1][j].tag == tag_content){
                                unit_lru = 1;
                                block_lru = j;
                        }
                        lru_state = 1;
                    }   
                }
            }
        }
        
        if(VC_Cache[unit_lru][block_lru].tag == tag_content){
            hit_pref_unit = unit_lru;
            hit_pref_block = block_lru;
            if(lru_state == 0){
                VC_Cache[0][0].unit_lru_order = 0;
                VC_Cache[1][0].unit_lru_order++;
                VC_Cache[2][0].unit_lru_order++;
            }else if(lru_state == 1){
                VC_Cache[1][0].unit_lru_order = 0;
                VC_Cache[0][0].unit_lru_order++;
                VC_Cache[2][0].unit_lru_order++;
            }else if(lru_state == 2){
                VC_Cache[2][0].unit_lru_order = 0;
                VC_Cache[0][0].unit_lru_order++;
                VC_Cache[1][0].unit_lru_order++;
            }
            return true;
        }else{
            return false;
        }*/
        /////////

        unsigned int unit_lru = lru_pos;
        unsigned int block_lru = 0;

        for(unsigned int i=0; i < Pref_Unit; i++){
            if(VC_Cache[i][0].unit_is_valid){
                for(unsigned int j=0; j < Pref_Size; j++){
                    if(VC_Cache[i][j].addr == addr){
                            hit_pref_unit = i;
                            hit_pref_block = j;
                            return true;
                    }
                }
            }
        }
        return false;


//
        /*unsigned int unit_lru = lru_pos;
        unsigned int block_lru = 0;

        for(unsigned int i=0; i < Pref_Unit; i++){
            if(VC_Cache[i][0].unit_is_valid){
                for(unsigned int j=0; j < Pref_Size; j++){
                    if(VC_Cache[i][j].tag == tag_content){
                        if(VC_Cache[i][0].unit_lru_order >= VC_Cache[unit_lru][0].unit_lru_order){
                            unit_lru = i;
                            block_lru = j;
                        }
                    }
                }
            }
        }

        if(VC_Cache[unit_lru][block_lru].tag == tag_content){
            hit_pref_unit = unit_lru;
            hit_pref_block = block_lru;
            return true;
        }else{
            return false;
        }
        return false;*/
        
    }else{
        if(VC_Cache[0][0].unit_is_valid){
            for(unsigned int j=0; j < Pref_Size; j++){
                if(VC_Cache[0][j]. addr ==addr){
                    hit_pref_block = j;
                    return true;
                }
            }
            return false;
        }else{
            return false;
        }
        
    }

}

void Cache::pref_miss(uint32_t addr, uint32_t unit_pos){
    
    

    for(unsigned int j=0; j < Pref_Size; j++){
        VC_Cache[Pref_Unit-1][j].addr =  addr + (1+j);
        VC_Cache[Pref_Unit-1][j].tag = VC_Cache[Pref_Unit-1][j].addr >> index_bits;;
        mem_traffic++;
        prefetch++;
    }
    
    
    VC_Cache[Pref_Unit-1][0].unit_is_valid = true;
    
    if((Pref_Unit-1) != 0){
        Swap(Pref_Unit-1);
    }
    
    
    
    /*if(Pref_Unit > 1){
        int lru_state = 0;
        if(VC_Cache[0][0].unit_lru_order == 2){
            for(unsigned int j=0; j < Pref_Size; j++){
                VC_Cache[0][j].addr =  addr + (1+j);
                VC_Cache[0][j].tag = VC_Cache[0][j].addr >> index_bits;;
                
            }
            VC_Cache[0][0].unit_is_valid = true;
            lru_state = 0;
        }else if(VC_Cache[1][0].unit_lru_order == 2){
            for(unsigned int j=0; j < Pref_Size; j++){
                VC_Cache[1][j].addr =  addr + (1+j);
                VC_Cache[1][j].tag = VC_Cache[1][j].addr >> index_bits;;
                
            }
            VC_Cache[1][0].unit_is_valid = true;
            lru_state = 1;
        }else if(VC_Cache[2][0].unit_lru_order == 2){
            for(unsigned int j=0; j < Pref_Size; j++){
                VC_Cache[2][j].addr =  addr + (1+j);
                VC_Cache[2][j].tag = VC_Cache[2][j].addr >> index_bits;;
                
            }
            VC_Cache[2][0].unit_is_valid = true;
            lru_state = 2;
        }

        if(lru_state == 0){
            VC_Cache[0][0].unit_lru_order = 0;
            VC_Cache[1][0].unit_lru_order++;
            VC_Cache[2][0].unit_lru_order++;
        }else if(lru_state == 1){
            VC_Cache[1][0].unit_lru_order = 0;
            VC_Cache[0][0].unit_lru_order++;
            VC_Cache[2][0].unit_lru_order++;
        }else if(lru_state == 2){
            VC_Cache[2][0].unit_lru_order = 0;
            VC_Cache[0][0].unit_lru_order++;
            VC_Cache[1][0].unit_lru_order++;
        }
        
        
        
        
        
        
        unsigned int last_lru_unit_pos = 0;
        for(unsigned int i=1; i < Pref_Unit; i++){
            if(VC_Cache[i][0].unit_lru_order > VC_Cache[last_lru_unit_pos][0].unit_lru_order){
                last_lru_unit_pos = i;
            }
        }
        for(unsigned int j=0; j < Pref_Size; j++){
            VC_Cache[last_lru_unit_pos][j].addr =  addr + (1+j);
            VC_Cache[last_lru_unit_pos][j].tag = VC_Cache[last_lru_unit_pos][j].addr >> index_bits;
        }

        VC_Cache[last_lru_unit_pos][0].unit_is_valid = true;
        vcblock_lru_upd(last_lru_unit_pos);*/
    
    
    /*}else{
        for(unsigned int j=0; j < Pref_Size; j++){
            VC_Cache[0][j].addr =  addr + (1+j);
            VC_Cache[0][j].tag = VC_Cache[0][j].addr >> index_bits;;
            
        }
        prefetch++;
        mem_traffic++;
        VC_Cache[0][0].unit_is_valid = true;
    }*/
    
}

void Cache::pref_contents_upd(uint32_t addr){    //uint32_t hit_pref_unit, uint32_t hit_pref_block
    if(Pref_Unit > 1){
        /*unsigned int a = Pref_Size - VC_Cache[hit_pref_unit][hit_pref_block].lru_order;
        for(unsigned int j=0; j < Pref_Size; j++){
            if(VC_Cache[hit_pref_unit][j].lru_order < VC_Cache[hit_pref_unit][hit_pref_block].lru_order){
                VC_Cache[hit_pref_unit][j].tag = VC_Cache[hit_pref_unit][hit_pref_block].tag + (a + VC_Cache[hit_pref_unit][j].lru_order);
                VC_Cache[hit_pref_unit][j].lru_order += (a-1);
            }
        }

        VC_Cache[hit_pref_unit][hit_pref_block].tag += Pref_Size;
        VC_Cache[hit_pref_unit][hit_pref_block].lru_order = Pref_Size - 1;

        vcblock_lru_upd(hit_pref_unit);*/
        
        uint32_t a = (hit_pref_block+1);
        for(unsigned int j=0; j < Pref_Size; j++){
            
            VC_Cache[hit_pref_unit][j].addr = addr + 1 + j;
            VC_Cache[hit_pref_unit][j].tag = VC_Cache[hit_pref_unit][j].addr >> index_bits;
            
        }
        mem_traffic += a;
        prefetch += a;
        if(hit_pref_unit != 0){
            Swap(hit_pref_unit);
        }
        //vcblock_lru_upd(hit_pref_unit);

    }else{
        /*unsigned int a = Pref_Size - VC_Cache[0][hit_pref_block].lru_order;
        for(unsigned int j=0; j < Pref_Size; j++){
            if(j != hit_pref_block){
                if(VC_Cache[0][j].lru_order < VC_Cache[0][hit_pref_block].lru_order){
                    VC_Cache[0][j].addr = VC_Cache[0][hit_pref_block].addr + (a + VC_Cache[0][j].lru_order);
                    
                    VC_Cache[0][j].lru_order += (a-1);
                }
            }
        }

        VC_Cache[0][hit_pref_block].addr += Pref_Size;
        prefetch++;
        VC_Cache[0][hit_pref_block].lru_order = Pref_Size - 1;*/
        uint32_t a = (hit_pref_block+1);
        for(unsigned int j=0; j < Pref_Size; j++){
            
            VC_Cache[0][j].addr = addr + 1 + j;
            VC_Cache[0][j].tag = VC_Cache[0][j].addr >> index_bits;
        }
        mem_traffic += a;
        prefetch += a;
    }

    /*for (unsigned int i = 0; i < Pref_Unit; i++) {
        if (i != hit_pref_unit) {
            if (VC_Cache[i].lru_order < VC_Cache[hit_pref_unit].lru_order) {
                VC_Cache[i].lru_order++;
            }
        }
    }
    VC_Cache[hit_pref_unit].lru_order = 0;*/
}

void Cache::vcblock_lru_upd(uint32_t unit_pos){    
    if(Pref_Unit > 1){
        for (unsigned int i = 0; i < Pref_Unit; i++) {
            if (i != unit_pos) {
                if (VC_Cache[i][0].unit_lru_order < VC_Cache[unit_pos][0].unit_lru_order) {
                    VC_Cache[i][0].unit_lru_order++;
                }
            }
        }
    }
    VC_Cache[unit_pos][0].unit_lru_order = 0;
}

void Cache::vc_to_c_transfer(uint32_t set_pos, uint32_t way_pos){
    if(Pref_Unit > 1){
        CacheBlock[set_pos][way_pos].tag = VC_Cache[hit_pref_unit][hit_pref_block].tag;
        CacheBlock[set_pos][way_pos].is_valid = true;
        CacheBlock[set_pos][way_pos].is_dirty = false;
    }else{
        CacheBlock[set_pos][way_pos].tag = VC_Cache[0][hit_pref_block].tag;
        CacheBlock[set_pos][way_pos].is_valid = true;
        CacheBlock[set_pos][way_pos].is_dirty = false;
    }
    //prefetch++;
}

void Cache::NL_Rdreq(char rw, uint32_t addr){
    if(N_last_level){
        Next_L_Ca->req(rw, addr);
    }else{
        mem_traffic++;
    }
}

void Cache::NL_Wrreq(char rw, uint32_t v_block_addr){
    if(V_BLock.is_valid && V_BLock.is_dirty){
        wr_back++;
        if(N_last_level){
            Next_L_Ca->req(rw, v_block_addr);
        }else{
            mem_traffic++;
        }
    }
}

void Cache::WA(uint32_t set_pos, uint32_t tag_content, uint32_t way_pos, bool need_wa, bool set_dirty){
    if(need_wa){
        CacheBlock[set_pos][way_pos].tag = tag_content;
        CacheBlock[set_pos][way_pos].is_valid = true;
        CacheBlock[set_pos][way_pos].is_dirty = false;
    }

    if(set_dirty){
        CacheBlock[set_pos][way_pos].is_dirty = true;
    }

    lru_update(set_pos, way_pos);

}

void Cache::lru_update(uint32_t set_pos, uint32_t way_pos) {
    if(is_asso) {
        for (unsigned int i = 0; i < Associativity; i++) {
            if (i != way_pos) {
                if (CacheBlock[set_pos][i].lru_order < CacheBlock[set_pos][way_pos].lru_order) {
                    CacheBlock[set_pos][i].lru_order++;
                }
            }
        }
    }
    CacheBlock[set_pos][way_pos].lru_order = 0;
}

void Cache::rd_req(uint32_t addr){

    uint32_t set_pos = addr & index_extract; 
    uint32_t tag_content = addr >> index_bits;
    Get_1st_LRU_Unit();
    Get_VC_Unit_Way();

    if(Block_Hit(set_pos, tag_content)){
        WA(set_pos, tag_content, hit_way_pos, 0, 0);
        
        if(Pref_Exist){
            if(!N_last_level){
                if(exist_vc(tag_content, first_lru_unit_pos, addr)){
                    pref_contents_upd(addr);
                }
            }
        }     
    }else{
        Get_V_Block_Way(set_pos, is_asso);
        V_BLock = CacheBlock[set_pos][v_block_way];
        v_block_addr = ((V_BLock.tag) << index_bits) | set_pos;
        NL_Wrreq('w', v_block_addr);
        
        if(!Pref_Exist){     //demand miss, no prefetch
            rd_miss++;
            NL_Rdreq('r', addr);      
            WA(set_pos, tag_content, v_block_way, 1, 0);
        }else{
            if(!N_last_level){
                if(!exist_vc(tag_content, first_lru_unit_pos, addr)){ // block is not exist in vc
                    rd_miss++;
                    NL_Rdreq('r', addr);      
                    WA(set_pos, tag_content, v_block_way, 1, 0);
                    
                    pref_miss(addr, last_lru_unit_pos);
                    
                }else{ // block exist in vc
                    vc_to_c_transfer(set_pos, v_block_way);
                    pref_contents_upd(addr);
                    WA(set_pos, tag_content, v_block_way, 0, 0);
                } 
            }else{
                rd_miss++;
                NL_Rdreq('r', addr);      
                WA(set_pos, tag_content, v_block_way, 1, 0);
            }
        }

    }

}

void Cache::wr_req(uint32_t addr){

    uint32_t set_pos = addr & index_extract; 
    uint32_t tag_content = addr >> index_bits;
    Get_1st_LRU_Unit();
    Get_VC_Unit_Way();
    
    if(Block_Hit(set_pos, tag_content)){
        WA(set_pos, tag_content, hit_way_pos, 0, 1);
        
        if(Pref_Exist){
            if(!N_last_level){
                if(exist_vc(tag_content, first_lru_unit_pos, addr)){
                    pref_contents_upd(addr);
                }
            }
        }     
    }else{
        Get_V_Block_Way(set_pos, is_asso);
        V_BLock = CacheBlock[set_pos][v_block_way];
        v_block_addr = ((V_BLock.tag) << index_bits) | set_pos;
        NL_Wrreq('w', v_block_addr);
        
        if(!Pref_Exist){     //demand miss, no prefetch
            wr_miss++;
            NL_Rdreq('r', addr);      
            WA(set_pos, tag_content, v_block_way, 1, 1);
        }else{
            if(!N_last_level){
                if(!exist_vc(tag_content, first_lru_unit_pos, addr)){ // block is not exist in vc
                    wr_miss++;
                    NL_Rdreq('r', addr);      
                    WA(set_pos, tag_content, v_block_way, 1, 1);

                    pref_miss(addr, last_lru_unit_pos); 
                }else{ // block exist in vc
                    vc_to_c_transfer(set_pos, v_block_way);
                    pref_contents_upd(addr);
                    WA(set_pos, tag_content, v_block_way, 0, 1);
                } 
            }else{
                wr_miss++;
                    NL_Rdreq('r', addr);      
                    WA(set_pos, tag_content, v_block_way, 1, 1);
            }
        }

    }
}

void Cache::req(char rw, uint32_t addr){
    if(rw == 'r'){
        rd++;
        rd_req(addr);
    }else if(rw == 'w'){
        wr++;
        wr_req(addr);
    }
}

/*void Cache::contents() {
    cout<< "\n===== "<<"L"<<cache_level<<" contents =====\n";
    for(unsigned int i=0; i < sets; i++) {
        cout <<"set\t"<<dec<< i <<":\t";
        for (unsigned int j = 0; j < Associativity; j++) {
            for (unsigned int k = 0; k < Associativity; k++) {
                if (CacheBlock[i][k].lru_order == j) {
                    char dirty_char=( CacheBlock[i][k].is_dirty == 1 )? 'D' : ' ';
                    cout <<" "<< hex << CacheBlock[i][k].tag << dec << " " << dirty_char << "\t";
                }
            }
        }
        cout << endl;
    }
	cout << endl;
}*/

void Cache::contents()
{
    cout<< "===== "<<"L"<<cache_level<<" contents =====\n";
    for (size_t i = 0; i < sets; i++) {
        if(i < 10){
            std::cout <<"set      "<<i<<":   ";
        }else if(i < 100){
            std::cout <<"set     "<<i<<":   ";
        }else if(i < 1000){
            std::cout <<"set    "<<i<<":   ";
        }
        
        for (unsigned int j = 0; j < Associativity; j++) {
            for (unsigned int k = 0; k < Associativity; k++) {
                if (CacheBlock[i][k].lru_order == j) {
                    char dirty_char=( CacheBlock[i][k].is_dirty == 1 )? 'D' : ' ';
                    if( CacheBlock[i][k].is_valid == 1 )
                    {
                        
                        if(cache_level == 1){
                            std::cout << std::hex << CacheBlock[i][k].tag << std::dec << " " << dirty_char <<"  ";
                        }else{
                            std::cout << " " << std::hex << CacheBlock[i][k].tag << std::dec << " " << dirty_char <<"   ";
                        }
                    }
                }
            }
        }
        std::cout << std::endl;
    }
    if(!N_last_level){
        if(Pref_Exist) {
            cout <<'\n';
            cout << "===== Stream Buffer(s) contents =====" << '\n';
            for (unsigned int i = 0; i < Pref_Unit; i++){
                for (unsigned int k = 0; k < Pref_Size; k++) {
                    for (unsigned int j = 0; j < Pref_Size; j++) {
                        if (VC_Cache[i][j].lru_order == k) {
                            cout << " " << hex << VC_Cache[i][j].addr << dec << " ";
                        }
                    }
                }
                cout <<'\n';
            }
        }
        std::cout << std::endl;
    }
    
}

