#ifndef AK_STD_INCLUDE
#define AK_STD_INCLUDE

#ifndef AK_ARENA_INITIAL_BLOCK_SIZE
#define AK_ARENA_INITIAL_BLOCK_SIZE (1024*1024)
#endif

#ifndef AK_DYNAMIC_ARRAY_INITIAL_CAPACITY
#define AK_DYNAMIC_ARRAY_INITIAL_CAPACITY 64
#endif

#ifndef AK_ARRAY_BUCKET_INITIAL_CAPACITY
#define AK_ARRAY_BUCKET_INITIAL_CAPACITY 64
#endif

#ifndef AK_HASH_MAP_INITIAL_SLOT_CAPACITY
#define AK_HASH_MAP_INITIAL_SLOT_CAPACITY 256
#endif

#ifndef AK_HASH_MAP_INITIAL_ITEM_CAPACITY
#define AK_HASH_MAP_INITIAL_ITEM_CAPACITY 64
#endif

#include <stdint.h>
#include <stdarg.h>

//~Raii definition
template <typename type>
struct ak_raii
{
    int32_t Cleanup;
    type*   ManagedResource;
    
    ak_raii(type* Resource);
    void Block();
    void Unblock();
    ~ak_raii();
};

struct ak_buffer
{
    uint8_t* Data;
    uint64_t Length;
};

//~Linked list macros
#define AK_DLL_Push_Back_NP(f,l,n,next,prev) ((f)==0?\
((f)=(l)=(n),(n)->next=(n)->prev=0):\
((n)->prev=(l),(l)->next=(n),(l)=(n),(n)->next=0))
#define AK_DLL_Push_Back(f,l,n) AK_DLL_Push_Back_NP(f,l,n,Next,Prev)

#define AK_DLL_Push_Front(f,l,n) AK_DLL_Push_Back_NP(l,f,n,Prev,Next)

#define AK_DLL_Remove_NP(f,l,n,next,prev) ((f)==(n)?\
((f)==(l)?\
((f)=(l)=(0)):\
((f)=(f)->next,(f)->prev=0)):\
(l)==(n)?\
((l)=(l)->prev,(l)->next=0):\
((n)->next->prev=(n)->prev,\
(n)->prev->next=(n)->next))
#define AK_DLL_Remove(f,l,n) AK_DLL_Remove_NP(f,l,n,Next,Prev)

#define AK_SLL_Queue_Push_N(f,l,n,next) ((f)==0?\
(f)=(l)=(n):\
((l)->next=(n),(l)=(n)),\
(n)->next=0)
#define AK_SLL_Queue_Push(f,l,n) AK_SLL_Queue_Push_N(f,l,n,Next)

#define AK_SLL_Queue_Push_Front_N(f,l,n,next) ((f)==0?\
((f)=(l)=(n),(n)->next=0):\
((n)->next=(f),(f)=(n)))
#define AK_SLL_Queue_Push_Front(f,l,n) AK_SLL_Queue_Push_Front_N(f,l,n,Next)

#define AK_SLL_Queue_Pop_N(f,l,next) ((f)==(l)?\
(f)=(l)=0:\
(f)=(f)->next)
#define AK_SLL_Queue_Pop(f,l) AK_SLL_Queue_Pop_N(f,l,Next)

#define AK_SLL_Stack_Push_N(f,n,next) ((n)->next=(f),(f)=(n))
#define AK_SLL_Stack_Push(f,n) AK_SLL_Stack_Push_N(f,n,Next)

#define AK_SLL_Stack_Pop_N(f,next) ((f)==0?0:\
(f)=(f)->next)
#define AK_SLL_Stack_Pop(f) AK_SLL_Stack_Pop_N(f,Next)

//~Array definition
template <typename type>
struct ak_array
{
    type*    Data   = NULL;
    uint64_t Length = 0;
    
    type* Get(uint64_t Index);
    const type* Get(uint64_t Index) const;
    
    type& operator[](uint64_t Index);
    const type& operator[](uint64_t Index) const;
    
    void Clear();
    
    type* begin();
    type* end();
};

#define AK_Expand_Array(array) (array), sizeof((array))/sizeof((array)[0])
template <typename type> ak_array<type> AK_Create_Array(type* Data, uint64_t Length);

//~Arena definition
enum ak_arena_clear_flag
{
    AK_ARENA_CLEAR,
    AK_ARENA_NO_CLEAR
};

struct ak__arena_block;

struct ak_arena_marker
{
    struct ak_arena* Arena;
    ak__arena_block* Block;
    uint64_t Marker;
};

struct ak_arena
{
    ak__arena_block* FirstBlock;  
    ak__arena_block* CurrentBlock;
    ak__arena_block* LastBlock;
    uint64_t     InitialBlockSize;
    
    ak_buffer Push(uint64_t Size, uint64_t Alignment, ak_arena_clear_flag ClearFlag = AK_ARENA_CLEAR);
    ak_buffer Push(uint64_t Size, ak_arena_clear_flag ClearFlag = AK_ARENA_CLEAR);
    
    template <typename type> type* Push_Struct(uint64_t Alignment, ak_arena_clear_flag ClearFlag = AK_ARENA_CLEAR);
    template <typename type> type* Push_Struct(ak_arena_clear_flag ClearFlag = AK_ARENA_CLEAR);
    
    template <typename type> ak_array<type> Push_Array(uint64_t Count, uint64_t Alignment, ak_arena_clear_flag ClearFlag = AK_ARENA_CLEAR);
    template <typename type> ak_array<type> Push_Array(uint64_t Count, ak_arena_clear_flag ClearFlag = AK_ARENA_CLEAR);
    
    ak_arena_marker Get_Marker();
    void Set_Marker(ak_arena_marker Marker);
    
    uint64_t Get_Total_Used() const;
    uint64_t Get_Total_Block_Size() const;
    
    void Clear(ak_arena_clear_flag ClearFlag = AK_ARENA_CLEAR);
};

struct ak_temp_arena
{
    ak_arena_marker Marker;
    
    ak_temp_arena(ak_arena* Arena);
    void Begin_Temp(ak_arena* Arena);
    void End_Temp();
    ~ak_temp_arena();
};

ak_arena* AK_Create_Arena(uint64_t InitialBlockSize = AK_ARENA_INITIAL_BLOCK_SIZE);
void AK_Delete(ak_arena* Arena);

//~Dynamic Array definition
template <typename type>
struct ak_dynamic_array : public ak_array<type>
{
    uint64_t Capacity = 0;
    
    bool Add(const type& Entry);
    bool Add_Range(const type* Entries, uint64_t Count);
    bool Add_Range(const type* Start, const type* End);
    
    bool Reserve(uint64_t NewCapacity);
    bool Resize(uint64_t NewLength);
};

template <typename type> ak_dynamic_array<type> AK_Create_Dynamic_Array(uint64_t InitialCapacity = AK_DYNAMIC_ARRAY_INITIAL_CAPACITY);
template <typename type> void AK_Delete(ak_dynamic_array<type>* Array);

//~Bucket array definition
template <typename type, uint64_t bucket_capacity>
struct ak_bucket_array;

template <typename type, uint64_t bucket_capacity>
struct ak__bucket
{
    type     Data[bucket_capacity];
    uint64_t Length;
};

template <typename type, uint64_t bucket_capacity>
struct ak_bucket_array_iterator
{
    ak_bucket_array<type, bucket_capacity>* Array;
    uint64_t CurrentBucketIndex;
    uint64_t CurrentIndexInBucket;
    
    type& operator*();
    void operator++();
    bool operator!=(const ak_bucket_array_iterator& Iterator);
};

template <typename type, uint64_t bucket_capacity = AK_ARRAY_BUCKET_INITIAL_CAPACITY>
struct ak_bucket_array
{
    ak_arena* Storage = NULL;
    ak_dynamic_array<ak__bucket<type, bucket_capacity>*> Buckets = {};
    uint64_t CurrentBucketIndex = 0;
    uint64_t Length = 0;
    
    bool Add(const type& Entry);
    bool Add_Range(const type* Entries, uint64_t Count);
    bool Add_Range(const type* Start, const type* End);
    
    type* Pop();
    
    type* Get(uint64_t Index);
    type& operator[](uint64_t Index);
    
    bool Resize(uint64_t NewLength);
    
    void Clear();
    
    ak_buffer Get_Buffer(ak_arena* Arena);
    ak_array<type> Get_Array(ak_arena* Arena);
    
    ak_bucket_array<type, bucket_capacity> Copy();
    
    ak_bucket_array_iterator<type, bucket_capacity> begin();
    ak_bucket_array_iterator<type, bucket_capacity> end();
};

template <typename type, uint64_t bucket_capacity> void AK_Delete(ak_bucket_array<type, bucket_capacity>* Array);

//~Hash Map definition
template <typename key, typename value>
struct ak_hashmap;

struct ak__hashmap_slot
{
    int32_t  IsValid = 0;
    uint32_t ItemIndex = 0;
    uint32_t BaseCount = 0;
    uint32_t Hash = 0;
};

template <typename key, typename value>
struct ak_hashmap_pair
{
    const key& Key;
    const value& Value;
};

template <typename key, typename value>
struct ak_hashmap_iterator
{
    const ak_hashmap<key, value>* HashMap;
    uint32_t CurrentIndex;
    
    ak_hashmap_pair<key, value> operator*();
    void operator++();
    bool operator!=(const ak_hashmap_iterator& Iterator);
};

template <typename key, typename value>
struct ak_hashmap
{
    uint32_t Length = 0;
    uint32_t SlotCapacity = 0;
    uint32_t ItemCapacity = 0;
    
    ak__hashmap_slot* Slots = NULL;
    uint32_t* ItemSlots = NULL;
    key* Keys = NULL;
    value* Values = NULL;
    
    void Add(const key& Key, const value& Value);
    value* Find(const key& Key);
    void Remove(const key& Key);
    void Clear();
    
    ak_hashmap_iterator<key, value> begin() const;
    ak_hashmap_iterator<key, value> end() const;
};

template <typename key, typename value> ak_hashmap<key, value>
AK_Create_Hash_Map(uint32_t InitialSlotCapacity=AK_HASH_MAP_INITIAL_SLOT_CAPACITY, uint32_t InitialItemCapacity=AK_HASH_MAP_INITIAL_ITEM_CAPACITY);

template <typename key, typename value> void
AK_Delete(ak_hashmap<key, value>* HashMap);

uint32_t AK_Hash_Function(uint32_t Key);
uint32_t AK_Hash_Function(int32_t Key);

uint32_t AK_Hash_Function(uint64_t Key);
uint32_t AK_Hash_Function(int64_t Key);

uint32_t AK_Hash_Function(float Key);
uint32_t AK_Hash_Function(double Key);

uint32_t AK_Hash_Function(const char* Key);

//~Pool definition
#define AK_POOL16_MAX_CAPACITY ((1 << 16)-1)

union ak__pool_id
{
    uint64_t ID;
    struct
    {
        uint32_t Key;
        struct
        {
            uint16_t PreviousIndex;
            uint16_t NextIndex;
        };
    };
    
    struct
    {
        uint32_t __Unused__;
        struct
        {
            uint16_t Type;
            uint16_t Index;
        };
    };
};

#define AK__INVALID_POOL_INDEX ((uint16_t)-1)

template <typename type>
struct ak__pool_entry
{
    ak__pool_id ID;
    type        Entry;
};

template <typename type, uint64_t bucket_capacity>
struct ak_pool;

template <typename type, uint64_t bucket_capacity>
struct ak_pool_iterator
{
    ak_pool<type, bucket_capacity>* Pool;
    uint16_t CurrentIndex;
    type& operator*();
    void operator++();
    bool operator!=(const ak_pool_iterator& Iterator);
};

template <typename type, uint64_t bucket_capacity = AK_ARRAY_BUCKET_INITIAL_CAPACITY>
struct ak_pool
{
    using ak_pool_storage = ak_bucket_array<ak__pool_entry<type>, bucket_capacity>;
    
    static const uint64_t MAX_CAPACITY = (1ull << 32)-1;
    
    ak_pool_storage Data;
    uint16_t Type = 0;
    uint16_t Length = 0;
    uint16_t FirstAllocatedIndex = AK__INVALID_POOL_INDEX;
    uint16_t FirstAvailableIndex = AK__INVALID_POOL_INDEX;
    uint32_t NextKey = 1;
    
    uint64_t Allocate(const type& Value = {});
    type* Allocate_And_Get(uint64_t* ID = NULL);
    void Free(uint64_t ID);
    void Free_All();
    
    bool Is_Allocated(uint64_t ID);
    type* Get(uint64_t ID);
    type& operator[](uint64_t ID);
    
    ak_pool_iterator<type, bucket_capacity> begin();
    ak_pool_iterator<type, bucket_capacity> end();
};

template <typename type, uint64_t bucket_capacity> 
void AK_Delete(ak_pool<type, bucket_capacity>* Pool);

//~String definition
#define AK_STR8_FIND_ERROR ((uint64_t)-1)

bool AK_Is_Newline(char Code);
bool AK_Is_Whitespace(char Code);
uint64_t AK_CStr_Length(const char* Str);

struct ak_str8
{
    const char* Str;
    uint64_t    Length;
    
    const char& operator[](uint64_t Index) const;
    
    bool Is_Null_Or_Empty();
    bool Is_Null_Or_Whitespace();
    
    uint64_t Find_First(char Code);
    uint64_t Find_Last(char Code);
    
    ak_str8 Prefix(uint64_t Count);
    ak_str8 Chop(uint64_t Count);
    ak_str8 Postfix(uint64_t Count);
    ak_str8 Skip(uint64_t Count);
    ak_str8 Substr(uint64_t First, uint64_t End);
    
    bool Begins_With(const ak_str8& CompStr);
    bool Ends_With(const ak_str8& CompStr);
    ak_str8 Trim_Whitespace();
    
    bool Is_Null_Term();
    const char* To_CStr(ak_arena* Arena);
    ak_str8 Copy(ak_arena* Arena);
};

struct ak_str8_node
{
    ak_str8_node* Next;
    ak_str8       String;
};

struct ak_str8_list
{
    ak_str8_node* First;
    ak_str8_node* Last;
    uint64_t      NodeCount;
    uint64_t      TotalLength;
    
    void Push_Explicit(const ak_str8& PushedString, ak_str8_node* Node);
    void Push(const ak_str8& PushedString, ak_arena* Arena);
    
    ak_str8 Join(ak_arena* Arena);
    ak_str8 Join_New_Line(ak_arena* Arena);
    
    void FormatV(ak_arena* Arena, const char* Format, va_list Args);
    void Format(ak_arena* Arena, const char* Format, ...);
};

ak_str8 AK_Str8(const char* Str);
ak_str8 AK_Str8(const char* Str, uint64_t StrLength);
ak_str8 AK_Str8(const char* First, const char* Last);
#define AK_Str8_Lit(s) AK_Str8((const char*)s, sizeof(s)-1)
ak_str8_list AK_Str8_Split(const ak_str8& String, ak_arena* Arena, char* SplitChars, uint32_t CharCount);
ak_str8 AK_Str8_FormatV(ak_arena* Arena, const char* Format, va_list Args);
ak_str8 AK_Str8_FormatV(ak_arena* Arena, const char* Format, ...);

bool operator!=(const ak_str8& A, const ak_str8& B);
bool operator==(const ak_str8& A, const ak_str8& B);

uint32_t AK_Hash_Function(const ak_str8& Str);

#endif //AK_STD_INCLUDE

#ifdef AK_STD_IMPLEMENTATION

#ifndef AK_STD_MALLOC
#include <stdlib.h>
#define AK_STD_MALLOC(size) malloc(size)
#define AK_STD_FREE(memory) free(memory)
#endif //AK_STD_MALLOC

#ifndef AK_STD_MEMCPY
#include <string.h>
#define AK_STD_MEMCPY(dest, src, n) memcpy(dest, src, n)
#define AK_STD_MEMSET(ptr, value, n) memset(ptr, value, n)
#endif //AK_STD_MEMCPY

#ifndef AK_STD_ASSERT
#include <assert.h>
#define AK_STD_ASSERT(condition, message_) assert(condition)
#endif

#define AK__Max(a, b) ((a) > (b) ? (a) : (b))
#define AK__Min(a, b) ((a) < (b) ? (a) : (b))

void AK__Memory_Copy(void* Dest, const void* Src, size_t Size)
{
    AK_STD_MEMCPY(Dest, Src, Size);
}

void AK__Memory_Set(void* Dest, uint8_t Value, size_t Size)
{
    AK_STD_MEMSET(Dest, Value, Size);
}

void AK__Memory_Clear(void* Dest, size_t Size)
{
    AK_STD_MEMSET(Dest, 0, Size);
}

void* AK__Malloc(size_t Size)
{
    return AK_STD_MALLOC(Size);
}

void AK__Free(void* Memory)
{
    if(Memory) AK_STD_FREE(Memory);
}

uint64_t AK__Memory_Align(uint64_t Value, uint64_t Alignment)
{
    Alignment = Alignment ? Alignment : 1;
    uint64_t Mod = Value & (Alignment-1);
    return Mod ? Value + (Alignment-Mod) : Value;
}

uint64_t AK__Ceil_Pow2(uint64_t V)
{
    --V;
    V |= V >> 1;
    V |= V >> 2;
    V |= V >> 4;
    V |= V >> 8;
    V |= V >> 16;
    V |= V >> 32;
    ++V;
    V += ( V == 0 );    
    return V;
}

//~Array implementation
template <typename type>
type* ak_array<type>::Get(uint64_t Index)
{
    if(Index >= Length) return NULL;
    return Data + Index;
}

template <typename type>
const type* ak_array<type>::Get(uint64_t Index) const
{
    if(Index >= Length) return NULL;
    return Data + Index;
}

template <typename type>
type& ak_array<type>::operator[](uint64_t Index)
{
    AK_STD_ASSERT(Index < Length, "Array index out of bounds!");
    return Data[Index];
}

template <typename type>
const type& ak_array<type>::operator[](uint64_t Index) const
{
    AK_STD_ASSERT(Index < Length, "Array index out of bounds!");
    return Data[Index];
}

template <typename type>
void ak_array<type>::Clear()
{
    AK__Memory_Clear(Data, sizeof(type)*Length);
    Length = 0;
}

template <typename type>
type* ak_array<type>::begin()
{
    return Data;
}

template <typename type>
type* ak_array<type>::end()
{
    return Data + Length;
}

template <typename type>
ak_array<type> AK_Create_Array(type* Ptr, uint64_t Length)
{
    ak_array<type> Result;
    Result.Data = Ptr;
    Result.Length = Length;
    return Result;
}

//~Raii implementation
template <typename type>
ak_raii<type>::ak_raii(type* Resource)
{
    ManagedResource = Resource;
    Cleanup = true;
}

template <typename type>
void ak_raii<type>::Block()
{
    Cleanup = false;
}

template <typename type>
void ak_raii<type>::Unblock()
{
    Cleanup = true;
}

template <typename type>
ak_raii<type>::~ak_raii()
{
    if(Cleanup) AK_Delete(ManagedResource);
}

//~Arena implementation

struct ak__arena_block
{
    uint8_t* Memory;
    uint64_t Used;
    uint64_t Size;
    ak__arena_block* Next;
};

ak__arena_block* AK__Arena_Allocate_Block(uint64_t BlockSize)
{
    ak__arena_block* Block = (ak__arena_block*)AK__Malloc(sizeof(ak__arena_block)+BlockSize);
    if(!Block)
    {
        //TODO(JJ): Diagnostic and error logging
        return NULL;
    }
    
    AK__Memory_Clear(Block, sizeof(ak__arena_block));
    Block->Memory = (uint8_t*)(Block+1);
    Block->Size = BlockSize;
    
    return Block;
}

void AK__Arena_Add_Block(ak_arena* Arena, ak__arena_block* Block)
{
    (Arena->FirstBlock == 0) ? 
    (Arena->FirstBlock = Arena->LastBlock = Block) : 
    (Arena->LastBlock->Next = Block, Arena->LastBlock = Block);
}

ak__arena_block* AK__Arena_Get_Block(ak_arena* Arena, uint64_t Size, uint64_t Alignment)
{
    ak__arena_block* Block = Arena->CurrentBlock;
    if(!Block) return NULL;
    
    uint64_t Used = AK__Memory_Align(Block->Used, Alignment);
    while(Used+Size > Block->Size)
    {
        Block = Block->Next;
        if(!Block) return NULL;
        Used = AK__Memory_Align(Block->Used, Alignment);
    }
    
    return Block;
}

ak_buffer ak_arena::Push(uint64_t Size, uint64_t Alignment, ak_arena_clear_flag ClearFlag)
{
    if(!Size) return {};
    
    ak__arena_block* Block = AK__Arena_Get_Block(this, Size, Alignment);
    if(!Block)
    {
        uint64_t BlockSize = InitialBlockSize;
        if(Size > BlockSize)
            BlockSize = (Alignment) ? AK__Memory_Align(Size, Alignment) : Size;
        
        Block = AK__Arena_Allocate_Block(BlockSize);
        if(!Block)
        {
            //TODO(JJ): Diagnostic and error logging
            return {};
        }
        
        AK__Arena_Add_Block(this, Block);
    }
    
    CurrentBlock = Block;
    if(Alignment) CurrentBlock->Used = AK__Memory_Align(CurrentBlock->Used, Alignment);
    
    uint8_t* Ptr = CurrentBlock->Memory + CurrentBlock->Used;
    CurrentBlock->Used += Size;
    
    if(ClearFlag == AK_ARENA_CLEAR)
        AK__Memory_Clear(Ptr, Size);
    
    ak_buffer Result;
    Result.Data = Ptr;
    Result.Length = Size;
    
    return Result;
}

ak_buffer ak_arena::Push(uint64_t Size, ak_arena_clear_flag ClearFlag)
{
    ak_buffer Result = Push(Size, 4, ClearFlag);
    return Result;
}

template <typename type> 
type* ak_arena::Push_Struct(uint64_t Alignment, ak_arena_clear_flag ClearFlag)
{
    return (type*)Push(sizeof(type), Alignment, ClearFlag).Data;
}

template <typename type> 
type* ak_arena::Push_Struct(ak_arena_clear_flag ClearFlag)
{
    return (type*)Push(sizeof(type), ClearFlag).Data;
}

template <typename type> 
ak_array<type> ak_arena::Push_Array(uint64_t Count, uint64_t Alignment, ak_arena_clear_flag ClearFlag)
{
    type* Ptr = (type*)Push(sizeof(type)*Count, Alignment, ClearFlag).Data;
    return AK_Create_Array<type>(Ptr, Count);
}

template <typename type> 
ak_array<type> ak_arena::Push_Array(uint64_t Count, ak_arena_clear_flag ClearFlag)
{
    type* Ptr = (type*)Push(sizeof(type)*Count, ClearFlag).Data; 
    return AK_Create_Array<type>(Ptr, Count);
}

ak_arena_marker ak_arena::Get_Marker()
{
    ak_arena_marker Marker;
    Marker.Arena = this;
    Marker.Block = CurrentBlock;
    if(CurrentBlock) Marker.Marker = CurrentBlock->Used;
    return Marker;
}

void ak_arena::Set_Marker(ak_arena_marker Marker)
{
    AK_STD_ASSERT(Marker.Arena == this, "Marker was not retrieved from this arena!");
    
    //NOTE(EVERYONE): If the block is null it always signalizes the beginning of the arena
    if(!Marker.Block)
    {
        CurrentBlock = FirstBlock;
        if(CurrentBlock) CurrentBlock->Used = 0;
    }
    else
    {
        CurrentBlock = Marker.Block;
        CurrentBlock->Used = Marker.Marker;
        for(ak__arena_block* ArenaBlock = CurrentBlock->Next; ArenaBlock; ArenaBlock = ArenaBlock->Next)
            ArenaBlock->Used = 0;
    }
}

uint64_t ak_arena::Get_Total_Used() const
{
    uint64_t Result = 0;
    for(ak__arena_block* Block = FirstBlock; Block; Block = Block->Next) Result += Block->Used;
    return Result;
}

uint64_t ak_arena::Get_Total_Block_Size() const
{
    uint64_t Result = 0;
    for(ak__arena_block* Block = FirstBlock; Block; Block = Block->Next) Result += Block->Size;
    return Result;
}

void ak_arena::Clear(ak_arena_clear_flag ClearFlag) 
{
    for(ak__arena_block* Block = FirstBlock; Block; Block = Block->Next)
    {
        if(ClearFlag == AK_ARENA_CLEAR) AK__Memory_Clear(Block->Memory, Block->Used);
        Block->Used = 0;
    }
}

ak_temp_arena::ak_temp_arena(ak_arena* TempArena)
{
    Begin_Temp(TempArena);
}

void ak_temp_arena::Begin_Temp(ak_arena* TempArena)
{
    Marker = TempArena->Get_Marker();
}

void ak_temp_arena::End_Temp()
{
    //NOTE(EVERYONE): If the block is null it always signalizes the beginning of the arena
    if(Marker.Arena)
    {
        Marker.Arena->Set_Marker(Marker);
        Marker.Arena = NULL;
    }
}

ak_temp_arena::~ak_temp_arena()
{
    End_Temp();
}

ak_arena* AK_Create_Arena(uint64_t InitialBlockSize)
{
    void* Memory = AK__Malloc(InitialBlockSize + sizeof(ak_arena)+sizeof(ak__arena_block));
    if(!Memory) 
    {
        //TODO(JJ): Diagnostic and error logging
        return NULL;
    }
    
    ak_arena* Result = (ak_arena*)Memory;
    Result->InitialBlockSize = InitialBlockSize;
    
    ak__arena_block* Block = (ak__arena_block*)(Result+1);
    AK__Memory_Clear(Block, sizeof(ak__arena_block));
    Block->Memory = (uint8_t*)(Block+1);
    AK__Arena_Add_Block(Result, Block);
    
    return Result;
}

void AK_Delete(ak_arena* Arena)
{
    if(Arena)
    {
        AK_STD_ASSERT(Arena->FirstBlock, "First block should've been allocated when the arena was created. This is a programming error");
        ak__arena_block* Block = Arena->FirstBlock->Next;
        while(Block)
        {
            ak__arena_block* BlockToDelete = Block;
            Block = Block->Next;
            AK__Free(BlockToDelete);
        }
        
        AK__Memory_Clear(Arena, sizeof(ak_arena));
    }
}

//~Dynamic Array implementation
template <typename type>
bool ak_dynamic_array<type>::Add(const type& Entry)
{
    if(!Capacity)
    {
        if(!Reserve(AK_DYNAMIC_ARRAY_INITIAL_CAPACITY))
        {
            //TODO(JJ): Diangostic and error logging
            return false;
        }
    }
    
    if(Length == Capacity)
    {
        if(!Reserve(Capacity*2))
        {
            //TODO(JJ): Diagnostic and error logging
            return false;
        }
    }
    
    Data[Length++] = Entry;
    return true;
}

template <typename type>
bool ak_dynamic_array<type>::Add_Range(const type* Entries, uint64_t Count)
{
    if(!Capacity)
    {
        uint64_t NewCapacity = AK__Max(Count, AK_DYNAMIC_ARRAY_INITIAL_CAPACITY);
        if(!Reserve(NewCapacity))
        {
            //TODO(JJ): Diagnostic and error logging
            return false;
        }
    }
    
    if(Length + Count > Capacity)
    {
        uint64_t NewCapacity = AK__Max(Length+Count, Capacity*2);
        if(!Reserve(NewCapacity))
        {
            //TODO(JJ): Diagnostic and error logging
            return false;
        }
    }
    
    AK__Memory_Copy(Data+Length, Entries, Count*sizeof(type));
    Length += Count;
    return true;
}

template <typename type>
bool ak_dynamic_array<type>::Add_Range(const type* Start, const type* End)
{
    return Add_Range(Start, End-Start);
}

template <typename type>
bool ak_dynamic_array<type>::Reserve(uint64_t NewCapacity)
{
    if(NewCapacity < Length) NewCapacity = Length;
    
    type* NewData = (type*)AK__Malloc(NewCapacity*sizeof(type));
    if(!NewData)
    {
        //TODO(JJ): Error handling
        return false;
    }
    
    if(Data) 
    {
        uint64_t CopySize = Capacity > NewCapacity ? NewCapacity : Capacity;
        AK__Memory_Copy(NewData, Data, CopySize*sizeof(type));
        AK__Free(Data);
    }
    
    Data = NewData;
    Capacity = NewCapacity;
    
    return true;
}

template <typename type>
bool ak_dynamic_array<type>::Resize(uint64_t NewLength)
{
    if(NewLength > Capacity) 
    {
        if(!Reserve(NewLength))
        {
            //TODO(JJ): Diagnostic and error logging
            return false;
        }
    }
    Length = NewLength;
    return true;
}

template <typename type> 
ak_dynamic_array<type> AK_Dynamic_Create_Array(uint64_t InitialCapacity)
{
    ak_dynamic_array<type> Result;
    Result.Reserve(InitialCapacity);
    return Result;
}

template <typename type>
void AK_Delete(ak_dynamic_array<type>* Array)
{
    if(Array && Array->Data)
    {
        AK__Free(Array->Data);
        AK__Memory_Clear(Array, sizeof(ak_dynamic_array<type>));
    }
}

//~Bucket Array implementation
template <typename type, uint64_t bucket_capacity>
type& ak_bucket_array_iterator<type, bucket_capacity>::operator*()
{
    return Array->Buckets[CurrentBucketIndex]->Data[CurrentIndexInBucket];
}

template <typename type, uint64_t bucket_capacity>
bool ak_bucket_array_iterator<type, bucket_capacity>::operator!=(const ak_bucket_array_iterator& Iterator)
{
    uint64_t Index = (CurrentBucketIndex*bucket_capacity) + CurrentIndexInBucket;
    return Index < Array->Length;
}

template <typename type, uint64_t bucket_capacity>
void ak_bucket_array_iterator<type, bucket_capacity>::operator++()
{
    CurrentIndexInBucket++;
    if(CurrentIndexInBucket == bucket_capacity)
    {
        CurrentBucketIndex++;
        CurrentIndexInBucket = 0;
    }
}

template <typename type, uint64_t bucket_capacity>
ak__bucket<type, bucket_capacity>* AK__Array_Get_Current_Bucket(ak_bucket_array<type, bucket_capacity>* Array)
{
    if(Array->CurrentBucketIndex == Array->Buckets.Length)
    {
        ak__bucket<type, bucket_capacity>* Bucket = Array->Storage->Push_Struct<ak__bucket<type, bucket_capacity>>();
        if(!Bucket)
        {
            //TODO(JJ): Diagnostic and error logging
            return NULL;
        }
        Bucket->Length = 0;
        Array->Buckets.Add(Bucket);
    }
    
    return Array->Buckets[Array->CurrentBucketIndex];
}

template <typename type, uint64_t bucket_capacity>
bool ak_bucket_array<type, bucket_capacity>::Add(const type& Entry)
{
    if(!Storage)
    {
        Storage = AK_Create_Arena();
        if(!Storage)
        {
            //TODO(JJ): Diagnostic and error logging
            return false;
        }
    }
    
    ak__bucket<type, bucket_capacity>* CurrentBucket = AK__Array_Get_Current_Bucket(this);
    if(!CurrentBucket)
    {
        //TODO(JJ): Diagnostic and error logging
        return false;
    }
    
    if(CurrentBucket->Length == bucket_capacity)
    {
        CurrentBucketIndex++;
        return Add(Entry);
    }
    
    CurrentBucket->Data[CurrentBucket->Length++] = Entry;
    Length++;
    return true;
}

template <typename type, uint64_t bucket_capacity>
bool ak_bucket_array<type, bucket_capacity>::Add_Range(const type* Entries, uint64_t Count)
{
    const type* EntryAt = Entries;
    while(Count--) 
    { 
        if(!Add(*EntryAt))
        {
            //TODO(JJ): Diagnostic and error logging
            return false;
        }
        EntryAt++; 
    }
    
    return true;
}

template <typename type, uint64_t bucket_capacity>
bool ak_bucket_array<type, bucket_capacity>::Add_Range(const type* Start, const type* End)
{
    return Add_Range(Start, End-Start);
}

template <typename type, uint64_t bucket_capacity>
type* ak_bucket_array<type, bucket_capacity>::Pop()
{
    if(!Length) return NULL;
    
    ak__bucket<type, bucket_capacity>* Bucket = Buckets[CurrentBucketIndex];
    AK_STD_ASSERT(Bucket->Length, "Invalid bucket count. Cannot pop a bucket with a length of 0");
    
    Bucket->Length--;
    type* Result = Bucket->Data + Bucket->Length;
    if(Bucket->Length == 0) CurrentBucketIndex = ((CurrentBucketIndex) ? (CurrentBucketIndex-1) : 0);
    Length--;
    return Result;
}

template <typename type, uint64_t bucket_capacity>
type* ak_bucket_array<type, bucket_capacity>::Get(uint64_t Index)
{
    uint64_t BucketIndex = Index / bucket_capacity;
    if(BucketIndex >= Buckets.Length) return NULL;
    
    uint64_t EntryIndex = Index % bucket_capacity;
    if(EntryIndex >= Buckets[BucketIndex]->Length) return NULL;
    
    return Buckets[BucketIndex]->Data + EntryIndex;
}

template <typename type, uint64_t bucket_capacity>
type& ak_bucket_array<type, bucket_capacity>::operator[](uint64_t Index)
{
    type* Result = Get(Index);
    AK_STD_ASSERT(Result, "Array out of bounds!");
    return *Result;
}

template <typename type, uint64_t bucket_capacity>
bool ak_bucket_array<type, bucket_capacity>::Resize(uint64_t NewLength)
{
    uint64_t NewLengthBucketIndex = NewLength / bucket_capacity;
    uint64_t NewLengthEntryIndex = NewLength % bucket_capacity;
    
    if(CurrentBucketIndex > NewLengthBucketIndex)
    {
        uint64_t Difference = CurrentBucketIndex-NewLengthBucketIndex;
        for(uint64_t BucketIndex = 0; BucketIndex < Difference; BucketIndex++)
        {
            Buckets[CurrentBucketIndex]->Length = 0;
            CurrentBucketIndex--;
        }
    }
    else if(CurrentBucketIndex < NewLengthBucketIndex)
    {
        uint64_t Difference = NewLengthBucketIndex-CurrentBucketIndex;
        for(uint64_t BucketIndex = 0; BucketIndex < Difference; BucketIndex++)
        {
            ak__bucket<type, bucket_capacity>* Bucket = AK__Array_Get_Current_Bucket(this);
            if(!Bucket)
            {
                //TODO(JJ): Diagnostic and error logging
                return false;
            }
            
            Bucket->Length = bucket_capacity;
            CurrentBucketIndex++;
        }
    }
    
    ak__bucket<type, bucket_capacity>* Bucket = AK__Array_Get_Current_Bucket(this);
    if(!Bucket)
    {
        //TODO(JJ): Diagnostic and error logging
        return false;
    }
    Bucket->Length = NewLengthEntryIndex;
    Length = NewLength;
    return true;
}

template <typename type, uint64_t bucket_capacity>
void ak_bucket_array<type, bucket_capacity>::Clear()
{
    for(uint64_t BucketIndex = 0; BucketIndex < Buckets.Length; BucketIndex++)
    {
        AK__Memory_Clear(Buckets[BucketIndex].Data, sizeof(type)*bucket_capacity);
        Buckets[BucketIndex].Length = 0;
    }
    Length = 0;
}

template <typename type, uint64_t bucket_capacity>
ak_buffer ak_bucket_array<type, bucket_capacity>::Get_Buffer(ak_arena* Arena)
{
    type* Memory = Arena->Push_Array<type>(Length);
    
    type* At = Memory;
    for(uint64_t BucketIndex = 0; BucketIndex < Buckets.Length; BucketIndex++)
    {
        ak__bucket<type, bucket_capacity>* Bucket = Buckets[BucketIndex];
        AK__Memory_Copy(At, Bucket->Data, Bucket->Length*sizeof(type));
        At += Bucket->Length;
    }
    
    ak_buffer Result = {};
    Result.Data = Memory;
    Result.Length = Length*sizeof(type);
    return Result;
}

template <typename type, uint64_t bucket_capacity>
ak_array<type> ak_bucket_array<type, bucket_capacity>::Get_Array(ak_arena* Arena)
{
    ak_buffer Buffer = Get_Buffer(Arena);
    ak_array<type> Result = AK_Create_Array<type>((type*)Buffer.Data, Length);
    return Result;
}

template <typename type, uint64_t bucket_capacity>
ak_bucket_array<type, bucket_capacity> ak_bucket_array<type, bucket_capacity>::Copy()
{
    ak_bucket_array<type, bucket_capacity> Result;
    Result.Storage = AK_Create_Arena();
    if(!Result.Storage)
    {
        //TODO(JJ): Diagnostic and error logging
        return {};
    }
    
    ak_raii<ak_bucket_array  <type, bucket_capacity>> RAII(&Result);
    Result.Buckets.Resize(Buckets.Length);
    
    for(uint64_t BucketIndex = 0; BucketIndex < Buckets.Length; BucketIndex++)
    {
        ak__bucket<type, bucket_capacity>* Bucket = Result.Storage->Push_Struct<ak__bucket<type, bucket_capacity>>();
        if(!Bucket)
        {
            //TODO(JJ): Diagnostic and error logging
            return {};
        }
        
        Bucket->Length = Buckets[BucketIndex]->Length;
        Result.Buckets[BucketIndex] = Bucket;
        AK__Memory_Copy(Bucket->Data, Buckets[BucketIndex]->Data, sizeof(type)*Buckets[BucketIndex]->Length);
    }
    
    Result.CurrentBucketIndex = CurrentBucketIndex;
    Result.Length = Length;
    
    RAII.Block();
    
    return Result;
}

template <typename type, uint64_t bucket_capacity>
ak_bucket_array_iterator<type, bucket_capacity> ak_bucket_array<type, bucket_capacity>::begin()
{
    ak_bucket_array_iterator<type, bucket_capacity> Result;
    Result.Array = this;
    Result.CurrentBucketIndex = 0;
    Result.CurrentIndexInBucket = 0;
    return Result;
}

template <typename type, uint64_t bucket_capacity>
ak_bucket_array_iterator<type, bucket_capacity> ak_bucket_array<type, bucket_capacity>::end()
{
    return {};
}

template <typename type, uint64_t bucket_capacity> 
void AK_Delete(ak_bucket_array<type, bucket_capacity>* Array)
{
    if(Array)
    {
        AK_Delete(&Array->Buckets);
        AK_Delete(Array->Storage);
        AK__Memory_Clear(Array, sizeof(ak_bucket_array<type, bucket_capacity>));
    }
}

//~Hash Map implementation
template <typename key, typename value>
ak_hashmap_pair<key, value> ak_hashmap_iterator<key, value>::operator*()
{
    ak_hashmap_pair<key, value> Result = 
    {
        HashMap->Keys[CurrentIndex], 
        HashMap->Values[CurrentIndex]
    };
    return Result;
}

template <typename key, typename value>
void ak_hashmap_iterator<key, value>::operator++()
{
    CurrentIndex++;
}

template <typename key, typename value>
bool ak_hashmap_iterator<key, value>::operator!=(const ak_hashmap_iterator& Iterator)
{
    return CurrentIndex != HashMap->Length;
}

ak__hashmap_slot* AK__HashMap_Realloc_Slots(ak__hashmap_slot* OldSlots, uint32_t* ItemSlots, uint32_t OldCapacity, uint32_t NewCapacity)
{
    uint32_t SlotMask = NewCapacity-1;
    
    uint64_t AllocSize = NewCapacity*sizeof(ak__hashmap_slot);
    ak__hashmap_slot* Slots = (ak__hashmap_slot*)AK__Malloc(AllocSize);
    AK__Memory_Clear(Slots, AllocSize);
    
    for(uint32_t OldSlotIndex = 0; OldSlotIndex < OldCapacity; OldSlotIndex++)
    {
        if(OldSlots[OldSlotIndex].IsValid)
        {
            uint32_t Hash = OldSlots[OldSlotIndex].Hash;
            uint32_t BaseSlot = Hash & SlotMask;
            uint32_t Slot = BaseSlot;
            while(Slots[Slot].IsValid)
                Slot = (Slot+1) & SlotMask;
            Slots[Slot].Hash = Hash;
            uint32_t ItemIndex = OldSlots[OldSlotIndex].ItemIndex;
            Slots[Slot].ItemIndex = ItemIndex;
            ItemSlots[ItemIndex] = Slot;            
            Slots[BaseSlot].BaseCount++;
            Slots[Slot].IsValid = true;
        }
    }
    
    AK__Free(OldSlots);
    return Slots;
}

template <typename key, typename value>
void AK__HashMap_Realloc(ak_hashmap<key, value>* Map)
{
    Map->ItemCapacity *= 2;
    
    uint64_t AllocSize = Map->ItemCapacity * (sizeof(key) + sizeof(value) + sizeof(uint32_t));
    void* MapData = AK__Malloc(AllocSize);        
    AK__Memory_Clear(MapData, AllocSize);
    
    uint32_t* NewItemSlots = (uint32_t*)MapData;
    key* NewKeys = (key*)(NewItemSlots + Map->ItemCapacity);
    value* NewValues = (value*)(NewKeys + Map->ItemCapacity);        
    
    AK__Memory_Copy(NewItemSlots, Map->ItemSlots, Map->Length*sizeof(uint32_t));
    AK__Memory_Copy(NewKeys, Map->Keys, Map->Length*sizeof(key));
    AK__Memory_Copy(NewValues, Map->Values, Map->Length*sizeof(value));
    AK__Free(Map->ItemSlots);
    
    Map->ItemSlots = NewItemSlots;
    Map->Keys = NewKeys;
    Map->Values = NewValues;        
}

template <typename key>
int64_t AK__HashMap_Find_Slot(key* Keys, ak__hashmap_slot* Slots, uint32_t SlotCapacity, const key& Key)
{
    uint32_t SlotMask = SlotCapacity - 1;
    uint32_t Hash = AK_Hash_Function(Key);
    
    uint32_t BaseSlot = Hash & SlotMask;
    uint32_t BaseCount = Slots[BaseSlot].BaseCount;
    uint32_t Slot = BaseSlot;
    
    while(BaseCount > 0)
    {
        if(Slots[Slot].IsValid)
        {
            uint32_t SlotHash = Slots[Slot].Hash;
            uint32_t SlotBase = SlotHash & SlotMask;
            if(SlotBase == BaseSlot)
            {
                BaseCount--;
                if(SlotHash == Hash && (Keys[Slots[Slot].ItemIndex] == Key))
                    return (int64_t)Slot;
            }
        }        
        
        Slot = (Slot+1) & SlotMask;
    }
    
    return -1;
}

template <typename key, typename value>
void ak_hashmap<key, value>::Add(const key& Key, const value& Value)
{
    if(!Slots || !ItemSlots)
        *this = AK_Create_Hash_Map<key, value>();
    
    AK_STD_ASSERT(AK__HashMap_Find_Slot(Keys, Slots, SlotCapacity, Key) < 0, "Cannot insert duplicate keys into hash map");
    
    if(Length >= (SlotCapacity - SlotCapacity/3))
    {
        uint32_t OldCapacity = SlotCapacity;
        SlotCapacity = (uint32_t)AK__Ceil_Pow2(SlotCapacity*2);
        Slots = AK__HashMap_Realloc_Slots(Slots, ItemSlots, OldCapacity, SlotCapacity);
    }
    
    uint32_t SlotMask = SlotCapacity-1;
    uint32_t Hash = AK_Hash_Function(Key);
    
    uint32_t BaseSlot = Hash & SlotMask;
    uint32_t BaseCount = Slots[BaseSlot].BaseCount;
    uint32_t Slot = BaseSlot;
    uint32_t FirstFree = Slot;
    
    while(BaseCount)
    {
        uint32_t SlotHash = Slots[Slot].Hash;
        if(!Slots[Slot].IsValid && Slots[FirstFree].IsValid) FirstFree = Slot;
        uint32_t SlotBase = SlotHash & SlotMask;
        if(SlotBase == BaseSlot)
            BaseCount--;
        Slot = (Slot+1) & SlotMask;
    }
    
    Slot = FirstFree;
    while(Slots[Slot].IsValid)
        Slot = (Slot+1) & SlotMask;
    
    if(Length >= ItemCapacity)
        AK__HashMap_Realloc(this);
    
    AK_STD_ASSERT(!Slots[Slot].IsValid && (Hash & SlotMask) == BaseSlot, "");
    AK_STD_ASSERT(Hash, "Invalid hash");
    
    Slots[Slot].Hash = Hash;
    Slots[Slot].ItemIndex = Length;
    Slots[Slot].IsValid = true;
    Slots[BaseSlot].BaseCount++;
    
    ItemSlots[Length] = Slot;
    Keys[Length] = Key;
    Values[Length] = Value;
    
    Length++;
}

template <typename key, typename value>
value* ak_hashmap<key, value>::Find(const key& Key)
{
    if(!Slots || !ItemSlots)
        *this = AK_Create_Hash_Map<key, value>();
    
    int64_t Slot = AK__HashMap_Find_Slot(Keys, Slots, SlotCapacity, Key);
    if(Slot < 0) return NULL;
    
    uint32_t Index = Slots[Slot].ItemIndex;
    return Values + Index;
}

template <typename key, typename value>
void ak_hashmap<key, value>::Remove(const key& Key)
{
    if(!Slots || !ItemSlots)
        *this = AK_Create_Hash_Map<key, value>();
    
    int64_t Slot = AK__HashMap_Find_Slot(Keys, Slots, SlotCapacity, Key);
    AK_STD_ASSERT(Slot >= 0, "Cannot find entry with key in hash map");
    AK_STD_ASSERT(Slots[Slot].IsValid, "Hash is invalid");
    
    if(Slot >= 0 && Slots[Slot].IsValid)
    {
        uint32_t SlotMask = SlotCapacity-1;
        uint32_t Hash = Slots[Slot].Hash;
        uint32_t BaseSlot = Hash & SlotMask;
        Slots[BaseSlot].BaseCount--;
        Slots[Slot].Hash = 0;
        Slots[Slot].IsValid = false;
        
        uint32_t Index = Slots[Slot].ItemIndex;
        uint32_t LastIndex = Length-1;
        
        if(Index != LastIndex)
        {
            Keys[Index] = Keys[LastIndex];
            ItemSlots[Index] = ItemSlots[LastIndex];
            Values[Index] = Values[LastIndex];
            Slots[ItemSlots[LastIndex]].ItemIndex = Index;
        }
        
        Length--;
    }
}

template <typename key, typename value>
void ak_hashmap<key, value>::Clear()
{
    AK__Memory_Clear(Slots, SlotCapacity*sizeof(ak__hashmap_slot));
    AK__Memory_Clear(ItemSlots, ItemCapacity*(sizeof(uint32_t)+sizeof(key)+sizeof(value)));
    Length = 0;
}

template <typename key, typename value>
ak_hashmap_iterator<key, value> ak_hashmap<key, value>::begin() const
{
    ak_hashmap_iterator<key, value> Result;
    Result.HashMap = this;
    Result.CurrentIndex = 0;
    return Result;
}

template <typename key, typename value>
ak_hashmap_iterator<key, value> ak_hashmap<key, value>::end() const
{
    return {};
}

template <typename key, typename value> 
ak_hashmap<key, value>  AK_Create_Hash_Map(uint32_t InitialSlotCapacity, uint32_t InitialItemCapacity)
{
    ak_hashmap<key, value> Result = {};
    Result.SlotCapacity = (uint32_t)AK__Ceil_Pow2(InitialSlotCapacity);
    Result.ItemCapacity = InitialItemCapacity;
    
    Result.Slots = (ak__hashmap_slot*)AK__Malloc(sizeof(ak__hashmap_slot)*Result.SlotCapacity);
    
    size_t AllocationSize = Result.ItemCapacity * (sizeof(key) + sizeof(value) + sizeof(uint32_t));
    void* MapData = AK__Malloc(AllocationSize);
    
    Result.ItemSlots = (uint32_t*)MapData;
    Result.Keys = (key*)(Result.ItemSlots + Result.ItemCapacity);
    Result.Values = (value*)(Result.Keys + Result.ItemCapacity);
    
    AK__Memory_Clear(Result.Slots, sizeof(ak__hashmap_slot)*Result.SlotCapacity);
    AK__Memory_Clear(MapData, AllocationSize);
    
    return Result;
}

template <typename key, typename value>
ak_hashmap<key, value> AK_Delete(ak_hashmap<key, value>* HashMap)
{
    if(HashMap)
    {
        AK__Free(HashMap->Slots);
        AK__Free(HashMap->ItemSlots);
    }
}

uint32_t AK_Hash_Function(uint32_t Key)
{
    Key = (Key+0x7ed55d16) + (Key<<12);
    Key = (Key^0xc761c23c) ^ (Key>>19);
    Key = (Key+0x165667b1) + (Key<<5);
    Key = (Key+0xd3a2646c) ^ (Key<<9);
    Key = (Key+0xfd7046c5) + (Key<<3);
    Key = (Key^0xb55a4f09) ^ (Key>>16);
    return Key;
}

uint32_t AK_Hash_Function(int32_t Key)
{
    return AK_Hash_Function((uint32_t)Key);
}

uint32_t AK_Hash_Function(uint64_t Key)
{
    Key = ( ~Key ) + ( Key << 18 );
    Key = Key ^ ( Key >> 31 );
    Key = Key * 21;
    Key = Key ^ ( Key >> 11 );
    Key = Key + ( Key << 6 );
    Key = Key ^ ( Key >> 22 );      
    return (uint32_t)Key;
}

uint32_t AK_Hash_Function(int64_t Key)
{
    return AK_Hash_Function((uint64_t)Key);
}

uint32_t AK_Hash_Function(float Key)
{
    union
    {
        float    F;
        uint32_t I;
    } U;
    
    U.F = Key;
    return AK_Hash_Function(U.I);
}

uint32_t AK_Hash_Function(double Key)
{
    union
    {
        double   D;
        uint64_t I;
    } U;
    
    U.D = Key;
    return AK_Hash_Function(U.I);
}

uint32_t AK_Hash_Function(const char* String)
{
    uint64_t Result = 0;
    uint64_t Rand1 = 31414;
    const uint64_t Rand2 = 27183;
    
    const char* At = String;
    while(*At)
    {
        Result *= Rand1;
        Result += *At;
        Rand1 *= Rand2;
        At++;
    }
    
    return AK_Hash_Function(Result);
}

//~Pool implementation

template <typename type, uint64_t bucket_capacity>
type& ak_pool_iterator<type, bucket_capacity>::operator*()
{
    return Pool->Data[CurrentIndex].Entry;
}

template <typename type, uint64_t bucket_capacity>
void ak_pool_iterator<type, bucket_capacity>::operator++()
{
    CurrentIndex = Pool->Data[CurrentIndex].ID.NextIndex;
}

template <typename type, uint64_t bucket_capacity>
bool ak_pool_iterator<type, bucket_capacity>::operator!=(const ak_pool_iterator& Iterator)
{
    return CurrentIndex != AK__INVALID_POOL_INDEX;
}

template <typename type, uint64_t bucket_capacity>
uint64_t ak_pool<type, bucket_capacity>::Allocate(const type& Value)
{
    uint16_t Index;
    if(FirstAvailableIndex != AK__INVALID_POOL_INDEX)
    {
        Index = FirstAvailableIndex;
        FirstAvailableIndex = Data[Index].ID.NextIndex;
    }
    else
    {
        Index = (uint16_t)Data.Length;
        if(!Data.Add({}))
        {
            //TODO(JJ): Diagnsotic and error logging
            return 0;
        }
    }
    
    AK_STD_ASSERT(Index < MAX_CAPACITY, "Pool capacity overflow!");
    if(Index >= MAX_CAPACITY)
    {
        //TODO(JJ): Diagnostic and error logging
        return 0;
    }
    
    ak__pool_entry<type>* Entry = &Data[Index];
    Entry->Entry = Value;
    Entry->ID.Key = NextKey++;
    
    if(!NextKey)
    {
        //NOTE(EVERYONE): We have overflowed the keys. Reset its position. All id entries that have had previous index values
        //will now be valid. This isn't super secure but hey, this really isn't the point of the pool data structure
        NextKey = 1;
    }
    
    if(FirstAllocatedIndex != AK__INVALID_POOL_INDEX)
    {
        ak__pool_entry<type>* FirstEntry = &Data[FirstAllocatedIndex];
        FirstEntry->ID.PreviousIndex = Index;
    }
    
    Entry->ID.PreviousIndex = AK__INVALID_POOL_INDEX;
    Entry->ID.NextIndex = FirstAllocatedIndex;
    
    FirstAllocatedIndex = Index;
    
    Length++;
    
    ak__pool_id Result;
    Result.Key = Entry->ID.Key;
    Result.Type = Type;
    Result.Index = Index;
    
    return Result.ID;
}

template <typename type, uint64_t bucket_capacity>
type* ak_pool<type, bucket_capacity>::Allocate_And_Get(uint64_t* OutID)
{
    ak__pool_id ID = {Allocate()};
    if(!ID.ID) return NULL;
    if(OutID) *OutID = ID.ID;
    return &Data[ID.Index].Entry;
}

template <typename type, uint64_t bucket_capacity>
void ak_pool<type, bucket_capacity>::Free(uint64_t TempID)
{
    if(Is_Allocated(TempID))
    {
        ak__pool_id ID = {TempID};
        ak__pool_entry<type>* Entry = &Data[ID.Index];
        
        bool IsHead = ID.Index == FirstAllocatedIndex;
        if(!IsHead)
        {
            AK_STD_ASSERT(Entry->ID.PreviousIndex != AK__INVALID_POOL_INDEX, "Cannot be the head of the pool and have a valid previous index");
            Data[Entry->ID.PreviousIndex].ID.NextIndex = Entry->ID.NextIndex;
        }
        else
            FirstAllocatedIndex = Entry->ID.NextIndex;
        
        if(Entry->ID.NextIndex != AK__INVALID_POOL_INDEX)
        {
            ak__pool_entry<type>* NextEntry = &Data[Entry->ID.NextIndex];
            NextEntry->ID.PreviousIndex = Entry->ID.PreviousIndex;
        }
        
        Entry->ID.ID = 0;
        Entry->ID.NextIndex = FirstAvailableIndex;
        
        FirstAvailableIndex = ID.Index;
        
        Length--;
    }
}

template <typename type, uint64_t bucket_capacity>
void ak_pool<type, bucket_capacity>::Free_All()
{
    if(Data) Data.Clear();
    Length = 0;
    FirstAllocatedIndex = 0;
    FirstAvailableIndex = 0;
}

template <typename type, uint64_t bucket_capacity>
bool ak_pool<type, bucket_capacity>::Is_Allocated(uint64_t TempID)
{
    if(TempID)
    {
        ak__pool_id ID = {TempID};
        ak__pool_entry<type>* Entry = &Data[ID.Index];
        bool Result = Entry->ID.Key && Entry->ID.Key == ID.Key;
        return Result;
    }
    return false;
}

template <typename type, uint64_t bucket_capacity>
type* ak_pool<type, bucket_capacity>::Get(uint64_t TempID)
{
    if(!Is_Allocated(TempID)) return NULL;
    ak__pool_id ID = {TempID};
    return &Data[ID.Index].Entry;
}

template <typename type, uint64_t bucket_capacity>
type& ak_pool<type, bucket_capacity>::operator[](uint64_t ID)
{
    type* Result = Get(ID);
    AK_STD_ASSERT(Result, "Cannot index into pool with invalid id");
    return *Result;
}

template <typename type, uint64_t bucket_capacity>
ak_pool_iterator<type, bucket_capacity> ak_pool<type, bucket_capacity>::begin()
{
    ak_pool_iterator<type, bucket_capacity> Result;
    Result.Pool = this;
    Result.CurrentIndex = FirstAllocatedIndex;
    return Result;
}

template <typename type, uint64_t bucket_capacity>
ak_pool_iterator<type, bucket_capacity> ak_pool<type, bucket_capacity>::end()
{
    return {};
}

template <typename type, uint64_t bucket_capacity> 
void AK_Delete(ak_pool<type, bucket_capacity>* Pool)
{
    if(Pool)
    {
        AK_Delete(&Pool->Data);
        AK__Memory_Clear(Pool, sizeof(ak_pool<type, bucket_capacity, id, index>));
    }
}

#define STB_SPRINTF_IMPLEMENTATION

// stb_sprintf - v1.10 - public domain snprintf() implementation
// originally by Jeff Roberts / RAD Game Tools, 2015/10/20
// http://github.com/nothings/stb
//
// allowed types:  sc uidBboXx p AaGgEef n
// lengths      :  hh h ll j z t I64 I32 I
//
// Contributors:
//    Fabian "ryg" Giesen (reformatting)
//    github:aganm (attribute format)
//
// Contributors (bugfixes):
//    github:d26435
//    github:trex78
//    github:account-login
//    Jari Komppa (SI suffixes)
//    Rohit Nirmal
//    Marcin Wojdyr
//    Leonard Ritter
//    Stefano Zanotti
//    Adam Allison
//    Arvid Gerstmann
//    Markus Kolb
//
// LICENSE:
//
//   See end of file for license information.

#ifndef STB_SPRINTF_H_INCLUDE
#define STB_SPRINTF_H_INCLUDE

/*
Single file sprintf replacement.

Originally written by Jeff Roberts at RAD Game Tools - 2015/10/20.
Hereby placed in public domain.

This is a full sprintf replacement that supports everything that
the C runtime sprintfs support, including float/double, 64-bit integers,
hex floats, field parameters (%*.*d stuff), length reads backs, etc.

Why would you need this if sprintf already exists?  Well, first off,
it's *much* faster (see below). It's also much smaller than the CRT
versions code-space-wise. We've also added some simple improvements
that are super handy (commas in thousands, callbacks at buffer full,
for example). Finally, the format strings for MSVC and GCC differ
for 64-bit integers (among other small things), so this lets you use
the same format strings in cross platform code.

It uses the standard single file trick of being both the header file
and the source itself. If you just include it normally, you just get
the header file function definitions. To get the code, you include
it from a C or C++ file and define STB_SPRINTF_IMPLEMENTATION first.

It only uses va_args macros from the C runtime to do it's work. It
does cast doubles to S64s and shifts and divides U64s, which does
drag in CRT code on most platforms.

It compiles to roughly 8K with float support, and 4K without.
As a comparison, when using MSVC static libs, calling sprintf drags
in 16K.

API:
====
int stbsp_sprintf( char * buf, char const * fmt, ... )
int stbsp_snprintf( char * buf, int count, char const * fmt, ... )
  Convert an arg list into a buffer.  stbsp_snprintf always returns
  a zero-terminated string (unlike regular snprintf).

int stbsp_vsprintf( char * buf, char const * fmt, va_list va )
int stbsp_vsnprintf( char * buf, int count, char const * fmt, va_list va )
  Convert a va_list arg list into a buffer.  stbsp_vsnprintf always returns
  a zero-terminated string (unlike regular snprintf).

int stbsp_vsprintfcb( STBSP_SPRINTFCB * callback, void * user, char * buf, char const * fmt, va_list va )
    typedef char * STBSP_SPRINTFCB( char const * buf, void * user, int len );
  Convert into a buffer, calling back every STB_SPRINTF_MIN chars.
  Your callback can then copy the chars out, print them or whatever.
  This function is actually the workhorse for everything else.
  The buffer you pass in must hold at least STB_SPRINTF_MIN characters.
    // you return the next buffer to use or 0 to stop converting

void stbsp_set_separators( char comma, char period )
  Set the comma and period characters to use.

FLOATS/DOUBLES:
===============
This code uses a internal float->ascii conversion method that uses
doubles with error correction (double-doubles, for ~105 bits of
precision).  This conversion is round-trip perfect - that is, an atof
of the values output here will give you the bit-exact double back.

One difference is that our insignificant digits will be different than
with MSVC or GCC (but they don't match each other either).  We also
don't attempt to find the minimum length matching float (pre-MSVC15
doesn't either).

If you don't need float or doubles at all, define STB_SPRINTF_NOFLOAT
and you'll save 4K of code space.

64-BIT INTS:
============
This library also supports 64-bit integers and you can use MSVC style or
GCC style indicators (%I64d or %lld).  It supports the C99 specifiers
for size_t and ptr_diff_t (%jd %zd) as well.

EXTRAS:
=======
Like some GCCs, for integers and floats, you can use a ' (single quote)
specifier and commas will be inserted on the thousands: "%'d" on 12345
would print 12,345.

For integers and floats, you can use a "$" specifier and the number
will be converted to float and then divided to get kilo, mega, giga or
tera and then printed, so "%$d" 1000 is "1.0 k", "%$.2d" 2536000 is
"2.53 M", etc. For byte values, use two $:s, like "%$$d" to turn
2536000 to "2.42 Mi". If you prefer JEDEC suffixes to SI ones, use three
$:s: "%$$$d" -> "2.42 M". To remove the space between the number and the
suffix, add "_" specifier: "%_$d" -> "2.53M".

In addition to octal and hexadecimal conversions, you can print
integers in binary: "%b" for 256 would print 100.

PERFORMANCE vs MSVC 2008 32-/64-bit (GCC is even slower than MSVC):
===================================================================
"%d" across all 32-bit ints (4.8x/4.0x faster than 32-/64-bit MSVC)
"%24d" across all 32-bit ints (4.5x/4.2x faster)
"%x" across all 32-bit ints (4.5x/3.8x faster)
"%08x" across all 32-bit ints (4.3x/3.8x faster)
"%f" across e-10 to e+10 floats (7.3x/6.0x faster)
"%e" across e-10 to e+10 floats (8.1x/6.0x faster)
"%g" across e-10 to e+10 floats (10.0x/7.1x faster)
"%f" for values near e-300 (7.9x/6.5x faster)
"%f" for values near e+300 (10.0x/9.1x faster)
"%e" for values near e-300 (10.1x/7.0x faster)
"%e" for values near e+300 (9.2x/6.0x faster)
"%.320f" for values near e-300 (12.6x/11.2x faster)
"%a" for random values (8.6x/4.3x faster)
"%I64d" for 64-bits with 32-bit values (4.8x/3.4x faster)
"%I64d" for 64-bits > 32-bit values (4.9x/5.5x faster)
"%s%s%s" for 64 char strings (7.1x/7.3x faster)
"...512 char string..." ( 35.0x/32.5x faster!)
*/

#if defined(__clang__)
#if defined(__has_feature) && defined(__has_attribute)
#if __has_feature(address_sanitizer)
#if __has_attribute(__no_sanitize__)
#define STBSP__ASAN __attribute__((__no_sanitize__("address")))
#elif __has_attribute(__no_sanitize_address__)
#define STBSP__ASAN __attribute__((__no_sanitize_address__))
#elif __has_attribute(__no_address_safety_analysis__)
#define STBSP__ASAN __attribute__((__no_address_safety_analysis__))
#endif
#endif
#endif
#elif defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
#if defined(__SANITIZE_ADDRESS__) && __SANITIZE_ADDRESS__
#define STBSP__ASAN __attribute__((__no_sanitize_address__))
#endif
#endif

#ifndef STBSP__ASAN
#define STBSP__ASAN
#endif

#ifdef STB_SPRINTF_STATIC
#define STBSP__PUBLICDEC static
#define STBSP__PUBLICDEF static STBSP__ASAN
#else
#ifdef __cplusplus
#define STBSP__PUBLICDEC extern "C"
#define STBSP__PUBLICDEF extern "C" STBSP__ASAN
#else
#define STBSP__PUBLICDEC extern
#define STBSP__PUBLICDEF STBSP__ASAN
#endif
#endif

#if defined(__has_attribute)
#if __has_attribute(format)
#define STBSP__ATTRIBUTE_FORMAT(fmt,va) __attribute__((format(printf,fmt,va)))
#endif
#endif

#ifndef STBSP__ATTRIBUTE_FORMAT
#define STBSP__ATTRIBUTE_FORMAT(fmt,va)
#endif

#ifdef _MSC_VER
#define STBSP__NOTUSED(v)  (void)(v)
#else
#define STBSP__NOTUSED(v)  (void)sizeof(v)
#endif

#ifndef STB_SPRINTF_MIN
#define STB_SPRINTF_MIN 512 // how many characters per callback
#endif
typedef char *STBSP_SPRINTFCB(const char *buf, void *user, int len);

#ifndef STB_SPRINTF_DECORATE
#define STB_SPRINTF_DECORATE(name) stbsp_##name // define this before including if you want to change the names
#endif

STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(vsprintf)(char *buf, char const *fmt, va_list va);
STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(vsnprintf)(char *buf, int count, char const *fmt, va_list va);
STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(sprintf)(char *buf, char const *fmt, ...) STBSP__ATTRIBUTE_FORMAT(2,3);
STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(snprintf)(char *buf, int count, char const *fmt, ...) STBSP__ATTRIBUTE_FORMAT(3,4);

STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(vsprintfcb)(STBSP_SPRINTFCB *callback, void *user, char *buf, char const *fmt, va_list va);
STBSP__PUBLICDEC void STB_SPRINTF_DECORATE(set_separators)(char comma, char period);

#endif // STB_SPRINTF_H_INCLUDE

#ifdef STB_SPRINTF_IMPLEMENTATION

#define stbsp__uint32 unsigned int
#define stbsp__int32 signed int

#ifdef _MSC_VER
#define stbsp__uint64 unsigned __int64
#define stbsp__int64 signed __int64
#else
#define stbsp__uint64 unsigned long long
#define stbsp__int64 signed long long
#endif
#define stbsp__uint16 unsigned short

#ifndef stbsp__uintptr
#if defined(__ppc64__) || defined(__powerpc64__) || defined(__aarch64__) || defined(_M_X64) || defined(__x86_64__) || defined(__x86_64) || defined(__s390x__)
#define stbsp__uintptr stbsp__uint64
#else
#define stbsp__uintptr stbsp__uint32
#endif
#endif

#ifndef STB_SPRINTF_MSVC_MODE // used for MSVC2013 and earlier (MSVC2015 matches GCC)
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define STB_SPRINTF_MSVC_MODE
#endif
#endif

#ifdef STB_SPRINTF_NOUNALIGNED // define this before inclusion to force stbsp_sprintf to always use aligned accesses
#define STBSP__UNALIGNED(code)
#else
#define STBSP__UNALIGNED(code) code
#endif

#ifndef STB_SPRINTF_NOFLOAT
// internal float utility functions
static stbsp__int32 stbsp__real_to_str(char const **start, stbsp__uint32 *len, char *out, stbsp__int32 *decimal_pos, double value, stbsp__uint32 frac_digits);
static stbsp__int32 stbsp__real_to_parts(stbsp__int64 *bits, stbsp__int32 *expo, double value);
#define STBSP__SPECIAL 0x7000
#endif

static char stbsp__period = '.';
static char stbsp__comma = ',';
static struct
{
    short temp; // force next field to be 2-byte aligned
    char pair[201];
} stbsp__digitpair =
{
    0,
    "00010203040506070809101112131415161718192021222324"
        "25262728293031323334353637383940414243444546474849"
        "50515253545556575859606162636465666768697071727374"
        "75767778798081828384858687888990919293949596979899"
};

STBSP__PUBLICDEF void STB_SPRINTF_DECORATE(set_separators)(char pcomma, char pperiod)
{
    stbsp__period = pperiod;
    stbsp__comma = pcomma;
}

#define STBSP__LEFTJUST 1
#define STBSP__LEADINGPLUS 2
#define STBSP__LEADINGSPACE 4
#define STBSP__LEADING_0X 8
#define STBSP__LEADINGZERO 16
#define STBSP__INTMAX 32
#define STBSP__TRIPLET_COMMA 64
#define STBSP__NEGATIVE 128
#define STBSP__METRIC_SUFFIX 256
#define STBSP__HALFWIDTH 512
#define STBSP__METRIC_NOSPACE 1024
#define STBSP__METRIC_1024 2048
#define STBSP__METRIC_JEDEC 4096

static void stbsp__lead_sign(stbsp__uint32 fl, char *sign)
{
    sign[0] = 0;
    if (fl & STBSP__NEGATIVE) {
        sign[0] = 1;
        sign[1] = '-';
    } else if (fl & STBSP__LEADINGSPACE) {
        sign[0] = 1;
        sign[1] = ' ';
    } else if (fl & STBSP__LEADINGPLUS) {
        sign[0] = 1;
        sign[1] = '+';
    }
}

static STBSP__ASAN stbsp__uint32 stbsp__strlen_limited(char const *s, stbsp__uint32 limit)
{
    char const * sn = s;
    
    // get up to 4-byte alignment
    for (;;) {
        if (((stbsp__uintptr)sn & 3) == 0)
            break;
        
        if (!limit || *sn == 0)
            return (stbsp__uint32)(sn - s);
        
        ++sn;
        --limit;
    }
    
    // scan over 4 bytes at a time to find terminating 0
    // this will intentionally scan up to 3 bytes past the end of buffers,
    // but becase it works 4B aligned, it will never cross page boundaries
    // (hence the STBSP__ASAN markup; the over-read here is intentional
    // and harmless)
    while (limit >= 4) {
        stbsp__uint32 v = *(stbsp__uint32 *)sn;
        // bit hack to find if there's a 0 byte in there
        if ((v - 0x01010101) & (~v) & 0x80808080UL)
            break;
        
        sn += 4;
        limit -= 4;
    }
    
    // handle the last few characters to find actual size
    while (limit && *sn) {
        ++sn;
        --limit;
    }
    
    return (stbsp__uint32)(sn - s);
}

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE(vsprintfcb)(STBSP_SPRINTFCB *callback, void *user, char *buf, char const *fmt, va_list va)
{
    static char hex[] = "0123456789abcdefxp";
    static char hexu[] = "0123456789ABCDEFXP";
    char *bf;
    char const *f;
    int tlen = 0;
    
    bf = buf;
    f = fmt;
    for (;;) {
        stbsp__int32 fw, pr, tz;
        stbsp__uint32 fl;
        
        // macros for the callback buffer stuff
#define stbsp__chk_cb_bufL(bytes)                        \
{                                                     \
int len = (int)(bf - buf);                         \
if ((len + (bytes)) >= STB_SPRINTF_MIN) {          \
tlen += len;                                    \
if (0 == (bf = buf = callback(buf, user, len))) \
goto done;                                   \
}                                                  \
}
#define stbsp__chk_cb_buf(bytes)    \
{                                \
if (callback) {               \
stbsp__chk_cb_bufL(bytes); \
}                             \
}
#define stbsp__flush_cb()                      \
{                                           \
stbsp__chk_cb_bufL(STB_SPRINTF_MIN - 1); \
} // flush if there is even one byte in the buffer
#define stbsp__cb_buf_clamp(cl, v)                \
cl = v;                                        \
if (callback) {                                \
int lg = STB_SPRINTF_MIN - (int)(bf - buf); \
if (cl > lg)                                \
cl = lg;                                 \
}
        
        // fast copy everything up to the next % (or end of string)
        for (;;) {
            while (((stbsp__uintptr)f) & 3) {
                schk1:
                if (f[0] == '%')
                    goto scandd;
                schk2:
                if (f[0] == 0)
                    goto endfmt;
                stbsp__chk_cb_buf(1);
                *bf++ = f[0];
                ++f;
            }
            for (;;) {
                // Check if the next 4 bytes contain %(0x25) or end of string.
                // Using the 'hasless' trick:
                // https://graphics.stanford.edu/~seander/bithacks.html#HasLessInWord
                stbsp__uint32 v, c;
                v = *(stbsp__uint32 *)f;
                c = (~v) & 0x80808080;
                if (((v ^ 0x25252525) - 0x01010101) & c)
                    goto schk1;
                if ((v - 0x01010101) & c)
                    goto schk2;
                if (callback)
                    if ((STB_SPRINTF_MIN - (int)(bf - buf)) < 4)
                    goto schk1;
#ifdef STB_SPRINTF_NOUNALIGNED
                if(((stbsp__uintptr)bf) & 3) {
                    bf[0] = f[0];
                    bf[1] = f[1];
                    bf[2] = f[2];
                    bf[3] = f[3];
                } else
#endif
                {
                    *(stbsp__uint32 *)bf = v;
                }
                bf += 4;
                f += 4;
            }
        }
        scandd:
        
        ++f;
        
        // ok, we have a percent, read the modifiers first
        fw = 0;
        pr = -1;
        fl = 0;
        tz = 0;
        
        // flags
        for (;;) {
            switch (f[0]) {
                // if we have left justify
                case '-':
                fl |= STBSP__LEFTJUST;
                ++f;
                continue;
                // if we have leading plus
                case '+':
                fl |= STBSP__LEADINGPLUS;
                ++f;
                continue;
                // if we have leading space
                case ' ':
                fl |= STBSP__LEADINGSPACE;
                ++f;
                continue;
                // if we have leading 0x
                case '#':
                fl |= STBSP__LEADING_0X;
                ++f;
                continue;
                // if we have thousand commas
                case '\'':
                fl |= STBSP__TRIPLET_COMMA;
                ++f;
                continue;
                // if we have kilo marker (none->kilo->kibi->jedec)
                case '$':
                if (fl & STBSP__METRIC_SUFFIX) {
                    if (fl & STBSP__METRIC_1024) {
                        fl |= STBSP__METRIC_JEDEC;
                    } else {
                        fl |= STBSP__METRIC_1024;
                    }
                } else {
                    fl |= STBSP__METRIC_SUFFIX;
                }
                ++f;
                continue;
                // if we don't want space between metric suffix and number
                case '_':
                fl |= STBSP__METRIC_NOSPACE;
                ++f;
                continue;
                // if we have leading zero
                case '0':
                fl |= STBSP__LEADINGZERO;
                ++f;
                goto flags_done;
                default: goto flags_done;
            }
        }
        flags_done:
        
        // get the field width
        if (f[0] == '*') {
            fw = va_arg(va, stbsp__uint32);
            ++f;
        } else {
            while ((f[0] >= '0') && (f[0] <= '9')) {
                fw = fw * 10 + f[0] - '0';
                f++;
            }
        }
        // get the precision
        if (f[0] == '.') {
            ++f;
            if (f[0] == '*') {
                pr = va_arg(va, stbsp__uint32);
                ++f;
            } else {
                pr = 0;
                while ((f[0] >= '0') && (f[0] <= '9')) {
                    pr = pr * 10 + f[0] - '0';
                    f++;
                }
            }
        }
        
        // handle integer size overrides
        switch (f[0]) {
            // are we halfwidth?
            case 'h':
            fl |= STBSP__HALFWIDTH;
            ++f;
            if (f[0] == 'h')
                ++f;  // QUARTERWIDTH
            break;
            // are we 64-bit (unix style)
            case 'l':
            fl |= ((sizeof(long) == 8) ? STBSP__INTMAX : 0);
            ++f;
            if (f[0] == 'l') {
                fl |= STBSP__INTMAX;
                ++f;
            }
            break;
            // are we 64-bit on intmax? (c99)
            case 'j':
            fl |= (sizeof(size_t) == 8) ? STBSP__INTMAX : 0;
            ++f;
            break;
            // are we 64-bit on size_t or ptrdiff_t? (c99)
            case 'z':
            fl |= (sizeof(ptrdiff_t) == 8) ? STBSP__INTMAX : 0;
            ++f;
            break;
            case 't':
            fl |= (sizeof(ptrdiff_t) == 8) ? STBSP__INTMAX : 0;
            ++f;
            break;
            // are we 64-bit (msft style)
            case 'I':
            if ((f[1] == '6') && (f[2] == '4')) {
                fl |= STBSP__INTMAX;
                f += 3;
            } else if ((f[1] == '3') && (f[2] == '2')) {
                f += 3;
            } else {
                fl |= ((sizeof(void *) == 8) ? STBSP__INTMAX : 0);
                ++f;
            }
            break;
            default: break;
        }
        
        // handle each replacement
        switch (f[0]) {
#define STBSP__NUMSZ 512 // big enough for e308 (with commas) or e-307
            char num[STBSP__NUMSZ];
            char lead[8];
            char tail[8];
            char *s;
            char const *h;
            stbsp__uint32 l, n, cs;
            stbsp__uint64 n64;
#ifndef STB_SPRINTF_NOFLOAT
            double fv;
#endif
            stbsp__int32 dp;
            char const *sn;
            
            case 's':
            // get the string
            s = va_arg(va, char *);
            if (s == 0)
                s = (char *)"null";
            // get the length, limited to desired precision
            // always limit to ~0u chars since our counts are 32b
            l = stbsp__strlen_limited(s, (pr >= 0) ? pr : ~0u);
            lead[0] = 0;
            tail[0] = 0;
            pr = 0;
            dp = 0;
            cs = 0;
            // copy the string in
            goto scopy;
            
            case 'c': // char
            // get the character
            s = num + STBSP__NUMSZ - 1;
            *s = (char)va_arg(va, int);
            l = 1;
            lead[0] = 0;
            tail[0] = 0;
            pr = 0;
            dp = 0;
            cs = 0;
            goto scopy;
            
            case 'n': // weird write-bytes specifier
            {
                int *d = va_arg(va, int *);
                *d = tlen + (int)(bf - buf);
            } break;
            
#ifdef STB_SPRINTF_NOFLOAT
            case 'A':              // float
            case 'a':              // hex float
            case 'G':              // float
            case 'g':              // float
            case 'E':              // float
            case 'e':              // float
            case 'f':              // float
            va_arg(va, double); // eat it
            s = (char *)"No float";
            l = 8;
            lead[0] = 0;
            tail[0] = 0;
            pr = 0;
            cs = 0;
            STBSP__NOTUSED(dp);
            goto scopy;
#else
            case 'A': // hex float
            case 'a': // hex float
            h = (f[0] == 'A') ? hexu : hex;
            fv = va_arg(va, double);
            if (pr == -1)
                pr = 6; // default is 6
            // read the double into a string
            if (stbsp__real_to_parts((stbsp__int64 *)&n64, &dp, fv))
                fl |= STBSP__NEGATIVE;
            
            s = num + 64;
            
            stbsp__lead_sign(fl, lead);
            
            if (dp == -1023)
                dp = (n64) ? -1022 : 0;
            else
                n64 |= (((stbsp__uint64)1) << 52);
            n64 <<= (64 - 56);
            if (pr < 15)
                n64 += ((((stbsp__uint64)8) << 56) >> (pr * 4));
            // add leading chars
            
#ifdef STB_SPRINTF_MSVC_MODE
            *s++ = '0';
            *s++ = 'x';
#else
            lead[1 + lead[0]] = '0';
            lead[2 + lead[0]] = 'x';
            lead[0] += 2;
#endif
            *s++ = h[(n64 >> 60) & 15];
            n64 <<= 4;
            if (pr)
                *s++ = stbsp__period;
            sn = s;
            
            // print the bits
            n = pr;
            if (n > 13)
                n = 13;
            if (pr > (stbsp__int32)n)
                tz = pr - n;
            pr = 0;
            while (n--) {
                *s++ = h[(n64 >> 60) & 15];
                n64 <<= 4;
            }
            
            // print the expo
            tail[1] = h[17];
            if (dp < 0) {
                tail[2] = '-';
                dp = -dp;
            } else
                tail[2] = '+';
            n = (dp >= 1000) ? 6 : ((dp >= 100) ? 5 : ((dp >= 10) ? 4 : 3));
            tail[0] = (char)n;
            for (;;) {
                tail[n] = '0' + dp % 10;
                if (n <= 3)
                    break;
                --n;
                dp /= 10;
            }
            
            dp = (int)(s - sn);
            l = (int)(s - (num + 64));
            s = num + 64;
            cs = 1 + (3 << 24);
            goto scopy;
            
            case 'G': // float
            case 'g': // float
            h = (f[0] == 'G') ? hexu : hex;
            fv = va_arg(va, double);
            if (pr == -1)
                pr = 6;
            else if (pr == 0)
                pr = 1; // default is 6
            // read the double into a string
            if (stbsp__real_to_str(&sn, &l, num, &dp, fv, (pr - 1) | 0x80000000))
                fl |= STBSP__NEGATIVE;
            
            // clamp the precision and delete extra zeros after clamp
            n = pr;
            if (l > (stbsp__uint32)pr)
                l = pr;
            while ((l > 1) && (pr) && (sn[l - 1] == '0')) {
                --pr;
                --l;
            }
            
            // should we use %e
            if ((dp <= -4) || (dp > (stbsp__int32)n)) {
                if (pr > (stbsp__int32)l)
                    pr = l - 1;
                else if (pr)
                    --pr; // when using %e, there is one digit before the decimal
                goto doexpfromg;
            }
            // this is the insane action to get the pr to match %g semantics for %f
            if (dp > 0) {
                pr = (dp < (stbsp__int32)l) ? l - dp : 0;
            } else {
                pr = -dp + ((pr > (stbsp__int32)l) ? (stbsp__int32) l : pr);
            }
            goto dofloatfromg;
            
            case 'E': // float
            case 'e': // float
            h = (f[0] == 'E') ? hexu : hex;
            fv = va_arg(va, double);
            if (pr == -1)
                pr = 6; // default is 6
            // read the double into a string
            if (stbsp__real_to_str(&sn, &l, num, &dp, fv, pr | 0x80000000))
                fl |= STBSP__NEGATIVE;
            doexpfromg:
            tail[0] = 0;
            stbsp__lead_sign(fl, lead);
            if (dp == STBSP__SPECIAL) {
                s = (char *)sn;
                cs = 0;
                pr = 0;
                goto scopy;
            }
            s = num + 64;
            // handle leading chars
            *s++ = sn[0];
            
            if (pr)
                *s++ = stbsp__period;
            
            // handle after decimal
            if ((l - 1) > (stbsp__uint32)pr)
                l = pr + 1;
            for (n = 1; n < l; n++)
                *s++ = sn[n];
            // trailing zeros
            tz = pr - (l - 1);
            pr = 0;
            // dump expo
            tail[1] = h[0xe];
            dp -= 1;
            if (dp < 0) {
                tail[2] = '-';
                dp = -dp;
            } else
                tail[2] = '+';
#ifdef STB_SPRINTF_MSVC_MODE
            n = 5;
#else
            n = (dp >= 100) ? 5 : 4;
#endif
            tail[0] = (char)n;
            for (;;) {
                tail[n] = '0' + dp % 10;
                if (n <= 3)
                    break;
                --n;
                dp /= 10;
            }
            cs = 1 + (3 << 24); // how many tens
            goto flt_lead;
            
            case 'f': // float
            fv = va_arg(va, double);
            doafloat:
            // do kilos
            if (fl & STBSP__METRIC_SUFFIX) {
                double divisor;
                divisor = 1000.0f;
                if (fl & STBSP__METRIC_1024)
                    divisor = 1024.0;
                while (fl < 0x4000000) {
                    if ((fv < divisor) && (fv > -divisor))
                        break;
                    fv /= divisor;
                    fl += 0x1000000;
                }
            }
            if (pr == -1)
                pr = 6; // default is 6
            // read the double into a string
            if (stbsp__real_to_str(&sn, &l, num, &dp, fv, pr))
                fl |= STBSP__NEGATIVE;
            dofloatfromg:
            tail[0] = 0;
            stbsp__lead_sign(fl, lead);
            if (dp == STBSP__SPECIAL) {
                s = (char *)sn;
                cs = 0;
                pr = 0;
                goto scopy;
            }
            s = num + 64;
            
            // handle the three decimal varieties
            if (dp <= 0) {
                stbsp__int32 i;
                // handle 0.000*000xxxx
                *s++ = '0';
                if (pr)
                    *s++ = stbsp__period;
                n = -dp;
                if ((stbsp__int32)n > pr)
                    n = pr;
                i = n;
                while (i) {
                    if ((((stbsp__uintptr)s) & 3) == 0)
                        break;
                    *s++ = '0';
                    --i;
                }
                while (i >= 4) {
                    *(stbsp__uint32 *)s = 0x30303030;
                    s += 4;
                    i -= 4;
                }
                while (i) {
                    *s++ = '0';
                    --i;
                }
                if ((stbsp__int32)(l + n) > pr)
                    l = pr - n;
                i = l;
                while (i) {
                    *s++ = *sn++;
                    --i;
                }
                tz = pr - (n + l);
                cs = 1 + (3 << 24); // how many tens did we write (for commas below)
            } else {
                cs = (fl & STBSP__TRIPLET_COMMA) ? ((600 - (stbsp__uint32)dp) % 3) : 0;
                if ((stbsp__uint32)dp >= l) {
                    // handle xxxx000*000.0
                    n = 0;
                    for (;;) {
                        if ((fl & STBSP__TRIPLET_COMMA) && (++cs == 4)) {
                            cs = 0;
                            *s++ = stbsp__comma;
                        } else {
                            *s++ = sn[n];
                            ++n;
                            if (n >= l)
                                break;
                        }
                    }
                    if (n < (stbsp__uint32)dp) {
                        n = dp - n;
                        if ((fl & STBSP__TRIPLET_COMMA) == 0) {
                            while (n) {
                                if ((((stbsp__uintptr)s) & 3) == 0)
                                    break;
                                *s++ = '0';
                                --n;
                            }
                            while (n >= 4) {
                                *(stbsp__uint32 *)s = 0x30303030;
                                s += 4;
                                n -= 4;
                            }
                        }
                        while (n) {
                            if ((fl & STBSP__TRIPLET_COMMA) && (++cs == 4)) {
                                cs = 0;
                                *s++ = stbsp__comma;
                            } else {
                                *s++ = '0';
                                --n;
                            }
                        }
                    }
                    cs = (int)(s - (num + 64)) + (3 << 24); // cs is how many tens
                    if (pr) {
                        *s++ = stbsp__period;
                        tz = pr;
                    }
                } else {
                    // handle xxxxx.xxxx000*000
                    n = 0;
                    for (;;) {
                        if ((fl & STBSP__TRIPLET_COMMA) && (++cs == 4)) {
                            cs = 0;
                            *s++ = stbsp__comma;
                        } else {
                            *s++ = sn[n];
                            ++n;
                            if (n >= (stbsp__uint32)dp)
                                break;
                        }
                    }
                    cs = (int)(s - (num + 64)) + (3 << 24); // cs is how many tens
                    if (pr)
                        *s++ = stbsp__period;
                    if ((l - dp) > (stbsp__uint32)pr)
                        l = pr + dp;
                    while (n < l) {
                        *s++ = sn[n];
                        ++n;
                    }
                    tz = pr - (l - dp);
                }
            }
            pr = 0;
            
            // handle k,m,g,t
            if (fl & STBSP__METRIC_SUFFIX) {
                char idx;
                idx = 1;
                if (fl & STBSP__METRIC_NOSPACE)
                    idx = 0;
                tail[0] = idx;
                tail[1] = ' ';
                {
                    if (fl >> 24) { // SI kilo is 'k', JEDEC and SI kibits are 'K'.
                        if (fl & STBSP__METRIC_1024)
                            tail[idx + 1] = "_KMGT"[fl >> 24];
                        else
                            tail[idx + 1] = "_kMGT"[fl >> 24];
                        idx++;
                        // If printing kibits and not in jedec, add the 'i'.
                        if (fl & STBSP__METRIC_1024 && !(fl & STBSP__METRIC_JEDEC)) {
                            tail[idx + 1] = 'i';
                            idx++;
                        }
                        tail[0] = idx;
                    }
                }
            };
            
            flt_lead:
            // get the length that we copied
            l = (stbsp__uint32)(s - (num + 64));
            s = num + 64;
            goto scopy;
#endif
            
            case 'B': // upper binary
            case 'b': // lower binary
            h = (f[0] == 'B') ? hexu : hex;
            lead[0] = 0;
            if (fl & STBSP__LEADING_0X) {
                lead[0] = 2;
                lead[1] = '0';
                lead[2] = h[0xb];
            }
            l = (8 << 4) | (1 << 8);
            goto radixnum;
            
            case 'o': // octal
            h = hexu;
            lead[0] = 0;
            if (fl & STBSP__LEADING_0X) {
                lead[0] = 1;
                lead[1] = '0';
            }
            l = (3 << 4) | (3 << 8);
            goto radixnum;
            
            case 'p': // pointer
            fl |= (sizeof(void *) == 8) ? STBSP__INTMAX : 0;
            pr = sizeof(void *) * 2;
            fl &= ~STBSP__LEADINGZERO; // 'p' only prints the pointer with zeros
            // fall through - to X
            
            case 'X': // upper hex
            case 'x': // lower hex
            h = (f[0] == 'X') ? hexu : hex;
            l = (4 << 4) | (4 << 8);
            lead[0] = 0;
            if (fl & STBSP__LEADING_0X) {
                lead[0] = 2;
                lead[1] = '0';
                lead[2] = h[16];
            }
            radixnum:
            // get the number
            if (fl & STBSP__INTMAX)
                n64 = va_arg(va, stbsp__uint64);
            else
                n64 = va_arg(va, stbsp__uint32);
            
            s = num + STBSP__NUMSZ;
            dp = 0;
            // clear tail, and clear leading if value is zero
            tail[0] = 0;
            if (n64 == 0) {
                lead[0] = 0;
                if (pr == 0) {
                    l = 0;
                    cs = 0;
                    goto scopy;
                }
            }
            // convert to string
            for (;;) {
                *--s = h[n64 & ((1 << (l >> 8)) - 1)];
                n64 >>= (l >> 8);
                if (!((n64) || ((stbsp__int32)((num + STBSP__NUMSZ) - s) < pr)))
                    break;
                if (fl & STBSP__TRIPLET_COMMA) {
                    ++l;
                    if ((l & 15) == ((l >> 4) & 15)) {
                        l &= ~15;
                        *--s = stbsp__comma;
                    }
                }
            };
            // get the tens and the comma pos
            cs = (stbsp__uint32)((num + STBSP__NUMSZ) - s) + ((((l >> 4) & 15)) << 24);
            // get the length that we copied
            l = (stbsp__uint32)((num + STBSP__NUMSZ) - s);
            // copy it
            goto scopy;
            
            case 'u': // unsigned
            case 'i':
            case 'd': // integer
            // get the integer and abs it
            if (fl & STBSP__INTMAX) {
                stbsp__int64 i64 = va_arg(va, stbsp__int64);
                n64 = (stbsp__uint64)i64;
                if ((f[0] != 'u') && (i64 < 0)) {
                    n64 = (stbsp__uint64)-i64;
                    fl |= STBSP__NEGATIVE;
                }
            } else {
                stbsp__int32 i = va_arg(va, stbsp__int32);
                n64 = (stbsp__uint32)i;
                if ((f[0] != 'u') && (i < 0)) {
                    n64 = (stbsp__uint32)-i;
                    fl |= STBSP__NEGATIVE;
                }
            }
            
#ifndef STB_SPRINTF_NOFLOAT
            if (fl & STBSP__METRIC_SUFFIX) {
                if (n64 < 1024)
                    pr = 0;
                else if (pr == -1)
                    pr = 1;
                fv = (double)(stbsp__int64)n64;
                goto doafloat;
            }
#endif
            
            // convert to string
            s = num + STBSP__NUMSZ;
            l = 0;
            
            for (;;) {
                // do in 32-bit chunks (avoid lots of 64-bit divides even with constant denominators)
                char *o = s - 8;
                if (n64 >= 100000000) {
                    n = (stbsp__uint32)(n64 % 100000000);
                    n64 /= 100000000;
                } else {
                    n = (stbsp__uint32)n64;
                    n64 = 0;
                }
                if ((fl & STBSP__TRIPLET_COMMA) == 0) {
                    do {
                        s -= 2;
                        *(stbsp__uint16 *)s = *(stbsp__uint16 *)&stbsp__digitpair.pair[(n % 100) * 2];
                        n /= 100;
                    } while (n);
                }
                while (n) {
                    if ((fl & STBSP__TRIPLET_COMMA) && (l++ == 3)) {
                        l = 0;
                        *--s = stbsp__comma;
                        --o;
                    } else {
                        *--s = (char)(n % 10) + '0';
                        n /= 10;
                    }
                }
                if (n64 == 0) {
                    if ((s[0] == '0') && (s != (num + STBSP__NUMSZ)))
                        ++s;
                    break;
                }
                while (s != o)
                    if ((fl & STBSP__TRIPLET_COMMA) && (l++ == 3)) {
                    l = 0;
                    *--s = stbsp__comma;
                    --o;
                } else {
                    *--s = '0';
                }
            }
            
            tail[0] = 0;
            stbsp__lead_sign(fl, lead);
            
            // get the length that we copied
            l = (stbsp__uint32)((num + STBSP__NUMSZ) - s);
            if (l == 0) {
                *--s = '0';
                l = 1;
            }
            cs = l + (3 << 24);
            if (pr < 0)
                pr = 0;
            
            scopy:
            // get fw=leading/trailing space, pr=leading zeros
            if (pr < (stbsp__int32)l)
                pr = l;
            n = pr + lead[0] + tail[0] + tz;
            if (fw < (stbsp__int32)n)
                fw = n;
            fw -= n;
            pr -= l;
            
            // handle right justify and leading zeros
            if ((fl & STBSP__LEFTJUST) == 0) {
                if (fl & STBSP__LEADINGZERO) // if leading zeros, everything is in pr
                {
                    pr = (fw > pr) ? fw : pr;
                    fw = 0;
                } else {
                    fl &= ~STBSP__TRIPLET_COMMA; // if no leading zeros, then no commas
                }
            }
            
            // copy the spaces and/or zeros
            if (fw + pr) {
                stbsp__int32 i;
                stbsp__uint32 c;
                
                // copy leading spaces (or when doing %8.4d stuff)
                if ((fl & STBSP__LEFTJUST) == 0)
                    while (fw > 0) {
                    stbsp__cb_buf_clamp(i, fw);
                    fw -= i;
                    while (i) {
                        if ((((stbsp__uintptr)bf) & 3) == 0)
                            break;
                        *bf++ = ' ';
                        --i;
                    }
                    while (i >= 4) {
                        *(stbsp__uint32 *)bf = 0x20202020;
                        bf += 4;
                        i -= 4;
                    }
                    while (i) {
                        *bf++ = ' ';
                        --i;
                    }
                    stbsp__chk_cb_buf(1);
                }
                
                // copy leader
                sn = lead + 1;
                while (lead[0]) {
                    stbsp__cb_buf_clamp(i, lead[0]);
                    lead[0] -= (char)i;
                    while (i) {
                        *bf++ = *sn++;
                        --i;
                    }
                    stbsp__chk_cb_buf(1);
                }
                
                // copy leading zeros
                c = cs >> 24;
                cs &= 0xffffff;
                cs = (fl & STBSP__TRIPLET_COMMA) ? ((stbsp__uint32)(c - ((pr + cs) % (c + 1)))) : 0;
                while (pr > 0) {
                    stbsp__cb_buf_clamp(i, pr);
                    pr -= i;
                    if ((fl & STBSP__TRIPLET_COMMA) == 0) {
                        while (i) {
                            if ((((stbsp__uintptr)bf) & 3) == 0)
                                break;
                            *bf++ = '0';
                            --i;
                        }
                        while (i >= 4) {
                            *(stbsp__uint32 *)bf = 0x30303030;
                            bf += 4;
                            i -= 4;
                        }
                    }
                    while (i) {
                        if ((fl & STBSP__TRIPLET_COMMA) && (cs++ == c)) {
                            cs = 0;
                            *bf++ = stbsp__comma;
                        } else
                            *bf++ = '0';
                        --i;
                    }
                    stbsp__chk_cb_buf(1);
                }
            }
            
            // copy leader if there is still one
            sn = lead + 1;
            while (lead[0]) {
                stbsp__int32 i;
                stbsp__cb_buf_clamp(i, lead[0]);
                lead[0] -= (char)i;
                while (i) {
                    *bf++ = *sn++;
                    --i;
                }
                stbsp__chk_cb_buf(1);
            }
            
            // copy the string
            n = l;
            while (n) {
                stbsp__int32 i;
                stbsp__cb_buf_clamp(i, n);
                n -= i;
                STBSP__UNALIGNED(while (i >= 4) {
                                     *(stbsp__uint32 volatile *)bf = *(stbsp__uint32 volatile *)s;
                                     bf += 4;
                                     s += 4;
                                     i -= 4;
                                 })
                    while (i) {
                    *bf++ = *s++;
                    --i;
                }
                stbsp__chk_cb_buf(1);
            }
            
            // copy trailing zeros
            while (tz) {
                stbsp__int32 i;
                stbsp__cb_buf_clamp(i, tz);
                tz -= i;
                while (i) {
                    if ((((stbsp__uintptr)bf) & 3) == 0)
                        break;
                    *bf++ = '0';
                    --i;
                }
                while (i >= 4) {
                    *(stbsp__uint32 *)bf = 0x30303030;
                    bf += 4;
                    i -= 4;
                }
                while (i) {
                    *bf++ = '0';
                    --i;
                }
                stbsp__chk_cb_buf(1);
            }
            
            // copy tail if there is one
            sn = tail + 1;
            while (tail[0]) {
                stbsp__int32 i;
                stbsp__cb_buf_clamp(i, tail[0]);
                tail[0] -= (char)i;
                while (i) {
                    *bf++ = *sn++;
                    --i;
                }
                stbsp__chk_cb_buf(1);
            }
            
            // handle the left justify
            if (fl & STBSP__LEFTJUST)
                if (fw > 0) {
                while (fw) {
                    stbsp__int32 i;
                    stbsp__cb_buf_clamp(i, fw);
                    fw -= i;
                    while (i) {
                        if ((((stbsp__uintptr)bf) & 3) == 0)
                            break;
                        *bf++ = ' ';
                        --i;
                    }
                    while (i >= 4) {
                        *(stbsp__uint32 *)bf = 0x20202020;
                        bf += 4;
                        i -= 4;
                    }
                    while (i--)
                        *bf++ = ' ';
                    stbsp__chk_cb_buf(1);
                }
            }
            break;
            
            default: // unknown, just copy code
            s = num + STBSP__NUMSZ - 1;
            *s = f[0];
            l = 1;
            fw = fl = 0;
            lead[0] = 0;
            tail[0] = 0;
            pr = 0;
            dp = 0;
            cs = 0;
            goto scopy;
        }
        ++f;
    }
    endfmt:
    
    if (!callback)
        *bf = 0;
    else
        stbsp__flush_cb();
    
    done:
    return tlen + (int)(bf - buf);
}

// cleanup
#undef STBSP__LEFTJUST
#undef STBSP__LEADINGPLUS
#undef STBSP__LEADINGSPACE
#undef STBSP__LEADING_0X
#undef STBSP__LEADINGZERO
#undef STBSP__INTMAX
#undef STBSP__TRIPLET_COMMA
#undef STBSP__NEGATIVE
#undef STBSP__METRIC_SUFFIX
#undef STBSP__NUMSZ
#undef stbsp__chk_cb_bufL
#undef stbsp__chk_cb_buf
#undef stbsp__flush_cb
#undef stbsp__cb_buf_clamp

// ============================================================================
//   wrapper functions

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE(sprintf)(char *buf, char const *fmt, ...)
{
    int result;
    va_list va;
    va_start(va, fmt);
    result = STB_SPRINTF_DECORATE(vsprintfcb)(0, 0, buf, fmt, va);
    va_end(va);
    return result;
}

typedef struct stbsp__context {
    char *buf;
    int count;
    int length;
    char tmp[STB_SPRINTF_MIN];
} stbsp__context;

static char *stbsp__clamp_callback(const char *buf, void *user, int len)
{
    stbsp__context *c = (stbsp__context *)user;
    c->length += len;
    
    if (len > c->count)
        len = c->count;
    
    if (len) {
        if (buf != c->buf) {
            const char *s, *se;
            char *d;
            d = c->buf;
            s = buf;
            se = buf + len;
            do {
                *d++ = *s++;
            } while (s < se);
        }
        c->buf += len;
        c->count -= len;
    }
    
    if (c->count <= 0)
        return c->tmp;
    return (c->count >= STB_SPRINTF_MIN) ? c->buf : c->tmp; // go direct into buffer if you can
}

static char * stbsp__count_clamp_callback( const char * buf, void * user, int len )
{
    stbsp__context * c = (stbsp__context*)user;
    (void) sizeof(buf);
    
    c->length += len;
    return c->tmp; // go direct into buffer if you can
}

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE( vsnprintf )( char * buf, int count, char const * fmt, va_list va )
{
    stbsp__context c;
    
    if ( (count == 0) && !buf )
    {
        c.length = 0;
        
        STB_SPRINTF_DECORATE( vsprintfcb )( stbsp__count_clamp_callback, &c, c.tmp, fmt, va );
    }
    else
    {
        int l;
        
        c.buf = buf;
        c.count = count;
        c.length = 0;
        
        STB_SPRINTF_DECORATE( vsprintfcb )( stbsp__clamp_callback, &c, stbsp__clamp_callback(0,&c,0), fmt, va );
        
        // zero-terminate
        l = (int)( c.buf - buf );
        if ( l >= count ) // should never be greater, only equal (or less) than count
            l = count - 1;
        buf[l] = 0;
    }
    
    return c.length;
}

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE(snprintf)(char *buf, int count, char const *fmt, ...)
{
    int result;
    va_list va;
    va_start(va, fmt);
    
    result = STB_SPRINTF_DECORATE(vsnprintf)(buf, count, fmt, va);
    va_end(va);
    
    return result;
}

STBSP__PUBLICDEF int STB_SPRINTF_DECORATE(vsprintf)(char *buf, char const *fmt, va_list va)
{
    return STB_SPRINTF_DECORATE(vsprintfcb)(0, 0, buf, fmt, va);
}

// =======================================================================
//   low level float utility functions

#ifndef STB_SPRINTF_NOFLOAT

// copies d to bits w/ strict aliasing (this compiles to nothing on /Ox)
#define STBSP__COPYFP(dest, src)                   \
{                                               \
int cn;                                      \
for (cn = 0; cn < 8; cn++)                   \
((char *)&dest)[cn] = ((char *)&src)[cn]; \
}

// get float info
static stbsp__int32 stbsp__real_to_parts(stbsp__int64 *bits, stbsp__int32 *expo, double value)
{
    double d;
    stbsp__int64 b = 0;
    
    // load value and round at the frac_digits
    d = value;
    
    STBSP__COPYFP(b, d);
    
    *bits = b & ((((stbsp__uint64)1) << 52) - 1);
    *expo = (stbsp__int32)(((b >> 52) & 2047) - 1023);
    
    return (stbsp__int32)((stbsp__uint64) b >> 63);
}

static double const stbsp__bot[23] = {
    1e+000, 1e+001, 1e+002, 1e+003, 1e+004, 1e+005, 1e+006, 1e+007, 1e+008, 1e+009, 1e+010, 1e+011,
    1e+012, 1e+013, 1e+014, 1e+015, 1e+016, 1e+017, 1e+018, 1e+019, 1e+020, 1e+021, 1e+022
};
static double const stbsp__negbot[22] = {
    1e-001, 1e-002, 1e-003, 1e-004, 1e-005, 1e-006, 1e-007, 1e-008, 1e-009, 1e-010, 1e-011,
    1e-012, 1e-013, 1e-014, 1e-015, 1e-016, 1e-017, 1e-018, 1e-019, 1e-020, 1e-021, 1e-022
};
static double const stbsp__negboterr[22] = {
    -5.551115123125783e-018,  -2.0816681711721684e-019, -2.0816681711721686e-020, -4.7921736023859299e-021, -8.1803053914031305e-022, 4.5251888174113741e-023,
    4.5251888174113739e-024,  -2.0922560830128471e-025, -6.2281591457779853e-026, -3.6432197315497743e-027, 6.0503030718060191e-028,  2.0113352370744385e-029,
    -3.0373745563400371e-030, 1.1806906454401013e-032,  -7.7705399876661076e-032, 2.0902213275965398e-033,  -7.1542424054621921e-034, -7.1542424054621926e-035,
    2.4754073164739869e-036,  5.4846728545790429e-037,  9.2462547772103625e-038,  -4.8596774326570872e-039
};
static double const stbsp__top[13] = {
    1e+023, 1e+046, 1e+069, 1e+092, 1e+115, 1e+138, 1e+161, 1e+184, 1e+207, 1e+230, 1e+253, 1e+276, 1e+299
};
static double const stbsp__negtop[13] = {
    1e-023, 1e-046, 1e-069, 1e-092, 1e-115, 1e-138, 1e-161, 1e-184, 1e-207, 1e-230, 1e-253, 1e-276, 1e-299
};
static double const stbsp__toperr[13] = {
    8388608,
    6.8601809640529717e+028,
    -7.253143638152921e+052,
    -4.3377296974619174e+075,
    -1.5559416129466825e+098,
    -3.2841562489204913e+121,
    -3.7745893248228135e+144,
    -1.7356668416969134e+167,
    -3.8893577551088374e+190,
    -9.9566444326005119e+213,
    6.3641293062232429e+236,
    -5.2069140800249813e+259,
    -5.2504760255204387e+282
};
static double const stbsp__negtoperr[13] = {
    3.9565301985100693e-040,  -2.299904345391321e-063,  3.6506201437945798e-086,  1.1875228833981544e-109,
    -5.0644902316928607e-132, -6.7156837247865426e-155, -2.812077463003139e-178,  -5.7778912386589953e-201,
    7.4997100559334532e-224,  -4.6439668915134491e-247, -6.3691100762962136e-270, -9.436808465446358e-293,
    8.0970921678014997e-317
};

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
static stbsp__uint64 const stbsp__powten[20] = {
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
    1000000000,
    10000000000,
    100000000000,
    1000000000000,
    10000000000000,
    100000000000000,
    1000000000000000,
    10000000000000000,
    100000000000000000,
    1000000000000000000,
    10000000000000000000U
};
#define stbsp__tento19th ((stbsp__uint64)1000000000000000000)
#else
static stbsp__uint64 const stbsp__powten[20] = {
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
    1000000000,
    10000000000ULL,
    100000000000ULL,
    1000000000000ULL,
    10000000000000ULL,
    100000000000000ULL,
    1000000000000000ULL,
    10000000000000000ULL,
    100000000000000000ULL,
    1000000000000000000ULL,
    10000000000000000000ULL
};
#define stbsp__tento19th (1000000000000000000ULL)
#endif

#define stbsp__ddmulthi(oh, ol, xh, yh)                            \
{                                                               \
double ahi = 0, alo, bhi = 0, blo;                           \
stbsp__int64 bt;                                             \
oh = xh * yh;                                                \
STBSP__COPYFP(bt, xh);                                       \
bt &= ((~(stbsp__uint64)0) << 27);                           \
STBSP__COPYFP(ahi, bt);                                      \
alo = xh - ahi;                                              \
STBSP__COPYFP(bt, yh);                                       \
bt &= ((~(stbsp__uint64)0) << 27);                           \
STBSP__COPYFP(bhi, bt);                                      \
blo = yh - bhi;                                              \
ol = ((ahi * bhi - oh) + ahi * blo + alo * bhi) + alo * blo; \
}

#define stbsp__ddtoS64(ob, xh, xl)          \
{                                        \
double ahi = 0, alo, vh, t;           \
ob = (stbsp__int64)xh;                \
vh = (double)ob;                      \
ahi = (xh - vh);                      \
t = (ahi - xh);                       \
alo = (xh - (ahi - t)) - (vh + t);    \
ob += (stbsp__int64)(ahi + alo + xl); \
}

#define stbsp__ddrenorm(oh, ol) \
{                            \
double s;                 \
s = oh + ol;              \
ol = ol - (s - oh);       \
oh = s;                   \
}

#define stbsp__ddmultlo(oh, ol, xh, xl, yh, yl) ol = ol + (xh * yl + xl * yh);

#define stbsp__ddmultlos(oh, ol, xh, yl) ol = ol + (xh * yl);

static void stbsp__raise_to_power10(double *ohi, double *olo, double d, stbsp__int32 power) // power can be -323 to +350
{
    double ph, pl;
    if ((power >= 0) && (power <= 22)) {
        stbsp__ddmulthi(ph, pl, d, stbsp__bot[power]);
    } else {
        stbsp__int32 e, et, eb;
        double p2h, p2l;
        
        e = power;
        if (power < 0)
            e = -e;
        et = (e * 0x2c9) >> 14; /* %23 */
        if (et > 13)
            et = 13;
        eb = e - (et * 23);
        
        ph = d;
        pl = 0.0;
        if (power < 0) {
            if (eb) {
                --eb;
                stbsp__ddmulthi(ph, pl, d, stbsp__negbot[eb]);
                stbsp__ddmultlos(ph, pl, d, stbsp__negboterr[eb]);
            }
            if (et) {
                stbsp__ddrenorm(ph, pl);
                --et;
                stbsp__ddmulthi(p2h, p2l, ph, stbsp__negtop[et]);
                stbsp__ddmultlo(p2h, p2l, ph, pl, stbsp__negtop[et], stbsp__negtoperr[et]);
                ph = p2h;
                pl = p2l;
            }
        } else {
            if (eb) {
                e = eb;
                if (eb > 22)
                    eb = 22;
                e -= eb;
                stbsp__ddmulthi(ph, pl, d, stbsp__bot[eb]);
                if (e) {
                    stbsp__ddrenorm(ph, pl);
                    stbsp__ddmulthi(p2h, p2l, ph, stbsp__bot[e]);
                    stbsp__ddmultlos(p2h, p2l, stbsp__bot[e], pl);
                    ph = p2h;
                    pl = p2l;
                }
            }
            if (et) {
                stbsp__ddrenorm(ph, pl);
                --et;
                stbsp__ddmulthi(p2h, p2l, ph, stbsp__top[et]);
                stbsp__ddmultlo(p2h, p2l, ph, pl, stbsp__top[et], stbsp__toperr[et]);
                ph = p2h;
                pl = p2l;
            }
        }
    }
    stbsp__ddrenorm(ph, pl);
    *ohi = ph;
    *olo = pl;
}

// given a float value, returns the significant bits in bits, and the position of the
//   decimal point in decimal_pos.  +/-INF and NAN are specified by special values
//   returned in the decimal_pos parameter.
// frac_digits is absolute normally, but if you want from first significant digits (got %g and %e), or in 0x80000000
static stbsp__int32 stbsp__real_to_str(char const **start, stbsp__uint32 *len, char *out, stbsp__int32 *decimal_pos, double value, stbsp__uint32 frac_digits)
{
    double d;
    stbsp__int64 bits = 0;
    stbsp__int32 expo, e, ng, tens;
    
    d = value;
    STBSP__COPYFP(bits, d);
    expo = (stbsp__int32)((bits >> 52) & 2047);
    ng = (stbsp__int32)((stbsp__uint64) bits >> 63);
    if (ng)
        d = -d;
    
    if (expo == 2047) // is nan or inf?
    {
        *start = (bits & ((((stbsp__uint64)1) << 52) - 1)) ? "NaN" : "Inf";
        *decimal_pos = STBSP__SPECIAL;
        *len = 3;
        return ng;
    }
    
    if (expo == 0) // is zero or denormal
    {
        if (((stbsp__uint64) bits << 1) == 0) // do zero
        {
            *decimal_pos = 1;
            *start = out;
            out[0] = '0';
            *len = 1;
            return ng;
        }
        // find the right expo for denormals
        {
            stbsp__int64 v = ((stbsp__uint64)1) << 51;
            while ((bits & v) == 0) {
                --expo;
                v >>= 1;
            }
        }
    }
    
    // find the decimal exponent as well as the decimal bits of the value
    {
        double ph, pl;
        
        // log10 estimate - very specifically tweaked to hit or undershoot by no more than 1 of log10 of all expos 1..2046
        tens = expo - 1023;
        tens = (tens < 0) ? ((tens * 617) / 2048) : (((tens * 1233) / 4096) + 1);
        
        // move the significant bits into position and stick them into an int
        stbsp__raise_to_power10(&ph, &pl, d, 18 - tens);
        
        // get full as much precision from double-double as possible
        stbsp__ddtoS64(bits, ph, pl);
        
        // check if we undershot
        if (((stbsp__uint64)bits) >= stbsp__tento19th)
            ++tens;
    }
    
    // now do the rounding in integer land
    frac_digits = (frac_digits & 0x80000000) ? ((frac_digits & 0x7ffffff) + 1) : (tens + frac_digits);
    if ((frac_digits < 24)) {
        stbsp__uint32 dg = 1;
        if ((stbsp__uint64)bits >= stbsp__powten[9])
            dg = 10;
        while ((stbsp__uint64)bits >= stbsp__powten[dg]) {
            ++dg;
            if (dg == 20)
                goto noround;
        }
        if (frac_digits < dg) {
            stbsp__uint64 r;
            // add 0.5 at the right position and round
            e = dg - frac_digits;
            if ((stbsp__uint32)e >= 24)
                goto noround;
            r = stbsp__powten[e];
            bits = bits + (r / 2);
            if ((stbsp__uint64)bits >= stbsp__powten[dg])
                ++tens;
            bits /= r;
        }
        noround:;
    }
    
    // kill long trailing runs of zeros
    if (bits) {
        stbsp__uint32 n;
        for (;;) {
            if (bits <= 0xffffffff)
                break;
            if (bits % 1000)
                goto donez;
            bits /= 1000;
        }
        n = (stbsp__uint32)bits;
        while ((n % 1000) == 0)
            n /= 1000;
        bits = n;
        donez:;
    }
    
    // convert to string
    out += 64;
    e = 0;
    for (;;) {
        stbsp__uint32 n;
        char *o = out - 8;
        // do the conversion in chunks of U32s (avoid most 64-bit divides, worth it, constant denomiators be damned)
        if (bits >= 100000000) {
            n = (stbsp__uint32)(bits % 100000000);
            bits /= 100000000;
        } else {
            n = (stbsp__uint32)bits;
            bits = 0;
        }
        while (n) {
            out -= 2;
            *(stbsp__uint16 *)out = *(stbsp__uint16 *)&stbsp__digitpair.pair[(n % 100) * 2];
            n /= 100;
            e += 2;
        }
        if (bits == 0) {
            if ((e) && (out[0] == '0')) {
                ++out;
                --e;
            }
            break;
        }
        while (out != o) {
            *--out = '0';
            ++e;
        }
    }
    
    *decimal_pos = tens;
    *start = out;
    *len = e;
    return ng;
}

#undef stbsp__ddmulthi
#undef stbsp__ddrenorm
#undef stbsp__ddmultlo
#undef stbsp__ddmultlos
#undef STBSP__SPECIAL
#undef STBSP__COPYFP

#endif // STB_SPRINTF_NOFLOAT

// clean up
#undef stbsp__uint16
#undef stbsp__uint32
#undef stbsp__int32
#undef stbsp__uint64
#undef stbsp__int64
#undef STBSP__UNALIGNED

#endif // STB_SPRINTF_IMPLEMENTATION

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/

bool AK_Is_Newline(char Code)
{
    return Code == '\n' || Code == '\r';
}

bool AK_Is_Whitespace(char Code)
{
    return AK_Is_Newline(Code) || Code == ' ' || Code == '\t' || Code == '\v' || Code == '\f';
}

uint64_t AK_CStr_Length(const char* Str)
{
    uint64_t Result = 0;
    while(*Str++) Result++;
    return Result;
}

const char& ak_str8::operator[](uint64_t Index) const
{
    return Str[Index];
}

bool ak_str8::Is_Null_Or_Empty()
{
    return !Str || !Length;
}

bool ak_str8::Is_Null_Or_Whitespace()
{
    if(Is_Null_Or_Empty())
        return true;
    
    for(uint64_t i = 0; i < Length; i++)
    {
        if(!AK_Is_Whitespace(Str[i]))
            return false;
    }
    
    return true;
}

uint64_t ak_str8::Find_First(char Code)
{
    const char* Start = Str;
    const char* End = Str+Length;
    
    while(Start != End)
    {
        if(*Start == Code)
            return (uint64_t)(Start - Str);
        Start++;
    }
    
    return AK_STR8_FIND_ERROR;
}

uint64_t ak_str8::Find_Last(char Code)
{
    const char* End = Str + (Length-1);
    while(End != Str-1)
    {
        if(*End == Code)
            return (uint64_t)(End-Str);
        
        End--;
    }
    
    return AK_STR8_FIND_ERROR;
}

ak_str8 ak_str8::Prefix(uint64_t Count)
{
    Count = AK__Min(Count, Length);
    ak_str8 Result = {Str, Count};
    return Result;
}

ak_str8 ak_str8::Chop(uint64_t Count)
{
    Count = AK__Min(Count, Length);
    uint64_t Remaining = Length - Count;
    ak_str8 Result = {Str, Remaining};
    return Result;
}

ak_str8 ak_str8::Postfix(uint64_t Count)
{
    Count = AK__Min(Count, Length);
    uint64_t Skip = Length - Count;
    ak_str8 Result = {Str+Skip, Count};
    return Result;
}

ak_str8 ak_str8::Skip(uint64_t Count)
{
    Count= AK__Min(Count, Length);
    uint64_t Remaining = Length-Count;
    ak_str8 Result = {Str + Count, Remaining};
    return Result;
}

ak_str8 ak_str8::Substr(uint64_t First, uint64_t End)
{
    End = AK__Min(End, Length);
    First = AK__Min(First, Length);
    ak_str8 Result = {Str+First, End-First};
    return Result;
}

bool ak_str8::Begins_With(const ak_str8& CompStr)
{
    if(Length < CompStr.Length)
        return false;
    
    for(uint64_t Index = 0; Index < CompStr.Length; Index++)
    {           
        if(Str[Index] != CompStr.Str[Index])
        {
            return false;
        }
    }    
    
    return true;
}

bool ak_str8::Ends_With(const ak_str8& CompStr)
{
    if(Length < CompStr.Length) return false;
    
    const char* StrLast = Str + Length-1;
    const char* CompStrLast = CompStr.Str + CompStr.Length-1;
    
    for(uint64_t Index = 0; Index < CompStr.Length; Index++)
    {
        if(*StrLast != *CompStrLast)
            return false;
        
        StrLast--;
        CompStrLast--;
    }
    
    return true;
}

ak_str8 ak_str8::Trim_Whitespace()
{
    ak_str8 Result = *this;
    
    while(AK_Is_Whitespace(*Result.Str) && Result.Length)
    {
        Result.Str++;
        Result.Length--;
    }
    
    while(Result.Length && AK_Is_Whitespace(Result.Str[Result.Length-1]))
        Result.Length--;
    
    return Result;
}

bool ak_str8::Is_Null_Term()
{
    return Str[Length] == 0;
}

const char* ak_str8::To_CStr(ak_arena* Arena)
{
    ak_array<char> Result = Arena->Push_Array<char>(Length+1);
    if(!Result.Data)
    {
        //TODO(JJ): Diagnostic and error logging
        return NULL;
    }
    
    Result[Length] = 0;
    AK__Memory_Copy(Result.Data, Str, Length);
    return Result.Data;
}

ak_str8 ak_str8::Copy(ak_arena* Arena)
{
    ak_str8 Result = AK_Str8(To_CStr(Arena), Length);
    return Result;
}

void ak_str8_list::Push_Explicit(const ak_str8& PushedString, ak_str8_node* Node)
{
    Node->String = PushedString;
    AK_SLL_Queue_Push(First, Last, Node);
    NodeCount++;
    TotalLength += PushedString.Length;
}

void ak_str8_list::Push(const ak_str8& PushedString, ak_arena* Arena)
{
    ak_str8_node* Node = Arena->Push_Struct<ak_str8_node>();
    Push_Explicit(PushedString, Node);
}

ak_str8 ak_str8_list::Join(ak_arena* Arena)
{
    ak_array<char> Str = Arena->Push_Array<char>(TotalLength+1);
    char* At = Str.Data;
    
    for(ak_str8_node* Node = First; Node; Node = Node->Next)
    {
        AK__Memory_Copy(At, Node->String.Str, Node->String.Length);
        At += Node->String.Length;
    }
    
    Str[TotalLength] = 0;
    return AK_Str8(Str.Data, TotalLength);
}

ak_str8 ak_str8_list::Join_New_Line(ak_arena* Arena)
{
    ak_array<char> Str = Arena->Push_Array<char>(TotalLength+NodeCount+1);
    char* At = Str.Data;
    
    for(ak_str8_node* Node = First; Node; Node = Node->Next)
    {
        AK__Memory_Copy(At, Node->String.Str, Node->String.Length);
        At += Node->String.Length;
        *At++ = '\n';
    }
    
    Str[TotalLength+NodeCount] = 0;
    return AK_Str8(Str.Data, TotalLength+NodeCount);
}

void ak_str8_list::FormatV(ak_arena* Arena, const char* Format, va_list Args)
{
    ak_str8 Result = AK_Str8_FormatV(Arena, Format, Args);
    Push(Result, Arena);
}

void ak_str8_list::Format(ak_arena* Arena, const char* Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    FormatV(Arena, Format, Args);
    va_end(Args);
}

ak_str8 AK_Str8(const char* Str)
{
    return AK_Str8(Str, AK_CStr_Length(Str));
}

ak_str8 AK_Str8(const char* Str, uint64_t StrLength)
{
    ak_str8 Result;
    Result.Str = Str;
    Result.Length = StrLength;
    return Result;
}

ak_str8 AK_Str8(const char* First, const char* Last)
{
    return AK_Str8(First, Last-First);
}

ak_str8_list AK_Str8_Split(const ak_str8& String, ak_arena* Arena, char* SplitChars, uint32_t CharCount)
{
    ak_str8_list Result = {};
    
    const char* StrAt = String.Str;
    const char* WordFirst = StrAt;
    const char* StrEnd = String.Str + String.Length;
    for(;StrAt < StrEnd; StrAt++)
    {
        char Byte = *StrAt;
        int32_t IsSplit = false;
        
        for(uint32_t Index = 0; Index < CharCount; Index++)
        {
            if(Byte == SplitChars[Index])
            {
                IsSplit = true;
                break;
            }
        }
        
        if(IsSplit)
        {
            if(WordFirst < StrAt) 
                Result.Push(AK_Str8(WordFirst, StrAt), Arena);
            WordFirst = StrAt + 1;
        }
    }
    
    if(WordFirst < StrAt) 
        Result.Push(AK_Str8(WordFirst, StrAt), Arena);
    
    return Result;
}

ak_str8 AK_Str8_FormatV(ak_arena* Arena, const char* Format, va_list Args)
{
    int ActualSize = stbsp_vsnprintf(NULL, 0, Format, Args);
    
    ak_array<char> Result = Arena->Push_Array<char>(ActualSize+1);
    stbsp_vsnprintf(Result.Data, ActualSize, Format, Args);
    Result[ActualSize] = 0;
    
    return AK_Str8(Result.Data, ActualSize);
}

ak_str8 AK_Str8_FormatV(ak_arena* Arena, const char* Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    ak_str8 Result = AK_Str8_FormatV(Arena, Format, Args);
    va_end(Args);
    return Result;
}

bool operator!=(const ak_str8& A, const ak_str8& B)
{
    if(A.Length != B.Length) return true;
    for(uint64_t Index = 0; Index < A.Length; Index++)
        if(A[Index] != B[Index]) return true;
    return false;
}

bool operator==(const ak_str8& A, const ak_str8& B)
{
    if(A.Length != B.Length) return false;
    for(uint64_t Index = 0; Index < A.Length; Index++)
        if(A[Index] != B[Index]) return false;
    return true;
}

uint32_t AK_Hash_Function(const ak_str8& Str)
{
    uint32_t Result = 0;
    uint32_t Rand1 = 31414;
    const uint32_t Rand2 = 27183;
    
    for(uint64_t Index = 0; Index < Str.Length; Index++)
    {
        Result *= Rand1;
        Result += Str[Index];
        Rand1 *= Rand2;
    }
    
    return Result;
}

#endif //AK_STD_IMPLEMENTATION

#ifdef AK_STD_TESTS

/*
   The latest version of this library is available on GitHub;
   https://github.com/sheredom/utest.h
*/

/*
   This is free and unencumbered software released into the public domain.

   Anyone is free to copy, modify, publish, use, compile, sell, or
   distribute this software, either in source code form or as a compiled
   binary, for any purpose, commercial or non-commercial, and by any
   means.

   In jurisdictions that recognize copyright laws, the author or authors
   of this software dedicate any and all copyright interest in the
   software to the public domain. We make this dedication for the benefit
   of the public at large and to the detriment of our heirs and
   successors. We intend this dedication to be an overt act of
   relinquishment in perpetuity of all present and future rights to this
   software under copyright law.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.

   For more information, please refer to <http://unlicense.org/>
*/

#ifndef SHEREDOM_UTEST_H_INCLUDED
#define SHEREDOM_UTEST_H_INCLUDED

#ifdef _MSC_VER
/*
   Disable warning about not inlining 'inline' functions.
*/
#pragma warning(disable : 4710)

/*
   Disable warning about inlining functions that are not marked 'inline'.
*/
#pragma warning(disable : 4711)

#if _MSC_VER > 1900
/*
  Disable warning about preprocessor macros not being defined in MSVC headers.
*/
#pragma warning(disable : 4668)

/*
  Disable warning about no function prototype given in MSVC headers.
*/
#pragma warning(disable : 4255)

/*
  Disable warning about pointer or reference to potentially throwing function.
*/
#pragma warning(disable : 5039)
#endif

#pragma warning(push, 1)
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1920)
typedef __int64 utest_int64_t;
typedef unsigned __int64 utest_uint64_t;
typedef unsigned __int32 utest_uint32_t;
#else
#include <stdint.h>
typedef int64_t utest_int64_t;
typedef uint64_t utest_uint64_t;
typedef uint32_t utest_uint32_t;
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#if defined(__cplusplus)
#define UTEST_C_FUNC extern "C"
#else
#define UTEST_C_FUNC
#endif

#if defined(_MSC_VER) \
|| defined(__MINGW64__) \
|| defined(__MINGW32__)

#if defined(__MINGW64__) || defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

// define UTEST_USE_OLD_QPC before #include "utest.h" to use old QueryPerformanceCounter
#ifndef UTEST_USE_OLD_QPC
#pragma warning(push, 0)
#include <Windows.h>
#pragma warning(pop)

typedef LARGE_INTEGER utest_large_integer;
#else
//use old QueryPerformanceCounter definitions (not sure is this needed in some edge cases or not)
//on Win7 with VS2015 these extern declaration cause "second C linkage of overloaded function not allowed" error
typedef union {
    struct {
        unsigned long LowPart;
        long HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        unsigned long LowPart;
        long HighPart;
    } u;
    utest_int64_t QuadPart;
} utest_large_integer;

UTEST_C_FUNC __declspec(dllimport) int __stdcall QueryPerformanceCounter(
                                                                         utest_large_integer *);
UTEST_C_FUNC __declspec(dllimport) int __stdcall QueryPerformanceFrequency(
                                                                           utest_large_integer *);

#if defined(__MINGW64__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif
#endif

#elif defined(__linux__) \
|| defined(__FreeBSD__) \
|| defined(__OpenBSD__) \
|| defined(__NetBSD__) \
|| defined(__DragonFly__) \
|| defined(__sun__) \
|| defined(__HAIKU__)
/*
   slightly obscure include here - we need to include glibc's features.h, but
   we don't want to just include a header that might not be defined for other
   c libraries like musl. Instead we include limits.h, which we know on all
   glibc distributions includes features.h
*/
#include <limits.h>

#if defined(__GLIBC__) && defined(__GLIBC_MINOR__)
#include <time.h>

#if ((2 < __GLIBC__) || ((2 == __GLIBC__) && (17 <= __GLIBC_MINOR__)))
/* glibc is version 2.17 or above, so we can just use clock_gettime */
#define UTEST_USE_CLOCKGETTIME
#else
#include <sys/syscall.h>
#include <unistd.h>
#endif
#else // Other libc implementations
#include <time.h>
#define UTEST_USE_CLOCKGETTIME
#endif

#elif defined(__APPLE__)
#include <mach/mach_time.h>
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1920)
#define UTEST_PRId64 "I64d"
#define UTEST_PRIu64 "I64u"
#else
#include <inttypes.h>

#define UTEST_PRId64 PRId64
#define UTEST_PRIu64 PRIu64
#endif

#if defined(__cplusplus)
#define UTEST_INLINE inline

#if defined(__clang__)
#define UTEST_INITIALIZER_BEGIN_DISABLE_WARNINGS                               \
_Pragma("clang diagnostic push")                                             \
_Pragma("clang diagnostic ignored \"-Wglobal-constructors\"")

#define UTEST_INITIALIZER_END_DISABLE_WARNINGS _Pragma("clang diagnostic pop")
#else
#define UTEST_INITIALIZER_BEGIN_DISABLE_WARNINGS
#define UTEST_INITIALIZER_END_DISABLE_WARNINGS
#endif

#define UTEST_INITIALIZER(f)                                                   \
struct f##_cpp_struct { f##_cpp_struct(); }; \
UTEST_INITIALIZER_BEGIN_DISABLE_WARNINGS static f##_cpp_struct f##_cpp_global UTEST_INITIALIZER_END_DISABLE_WARNINGS; \
f##_cpp_struct::f##_cpp_struct()
#elif defined(_MSC_VER)
#define UTEST_INLINE __forceinline

#if defined(_WIN64)
#define UTEST_SYMBOL_PREFIX
#else
#define UTEST_SYMBOL_PREFIX "_"
#endif

#if defined(__clang__)
#define UTEST_INITIALIZER_BEGIN_DISABLE_WARNINGS                               \
_Pragma("clang diagnostic push")                                             \
_Pragma("clang diagnostic ignored \"-Wmissing-variable-declarations\"")

#define UTEST_INITIALIZER_END_DISABLE_WARNINGS _Pragma("clang diagnostic pop")
#else
#define UTEST_INITIALIZER_BEGIN_DISABLE_WARNINGS
#define UTEST_INITIALIZER_END_DISABLE_WARNINGS
#endif

#pragma section(".CRT$XCU", read)
#define UTEST_INITIALIZER(f)                                                   \
static void __cdecl f(void);                                                 \
UTEST_INITIALIZER_BEGIN_DISABLE_WARNINGS                                     \
__pragma(comment(linker, "/include:" UTEST_SYMBOL_PREFIX #f "_"))            \
UTEST_C_FUNC __declspec(allocate(".CRT$XCU")) void(__cdecl *             \
f##_)(void) = f;      \
UTEST_INITIALIZER_END_DISABLE_WARNINGS                                       \
static void __cdecl f(void)
#else
#if defined(__linux__)
#if defined(__clang__)
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#endif

#define __STDC_FORMAT_MACROS 1

#if defined(__clang__)
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic pop
#endif
#endif
#endif

#define UTEST_INLINE inline

#define UTEST_INITIALIZER(f)                                                   \
static void f(void) __attribute__((constructor));                            \
static void f(void)
#endif

#if defined(__cplusplus)
#define UTEST_CAST(type, x) static_cast<type>(x)
#define UTEST_PTR_CAST(type, x) reinterpret_cast<type>(x)
#define UTEST_EXTERN extern "C"
#define UTEST_NULL NULL
#else
#define UTEST_CAST(type, x) ((type)(x))
#define UTEST_PTR_CAST(type, x) ((type)(x))
#define UTEST_EXTERN extern
#define UTEST_NULL 0
#endif

#ifdef _MSC_VER
/*
    io.h contains definitions for some structures with natural padding. This is
    uninteresting, but for some reason MSVC's behaviour is to warn about
    including this system header. That *is* interesting
*/
#pragma warning(disable : 4820)
#pragma warning(push, 1)
#include <io.h>
#pragma warning(pop)
#define UTEST_COLOUR_OUTPUT() (_isatty(_fileno(stdout)))
#else
#if  defined(__EMSCRIPTEN__)
#include <emscripten/html5.h>
#define UTEST_COLOUR_OUTPUT() false
#else
#include <unistd.h>
#define UTEST_COLOUR_OUTPUT() (isatty(STDOUT_FILENO))
#endif
#endif

static UTEST_INLINE void *utest_realloc(void *const pointer, size_t new_size) {
    void *const new_pointer = realloc(pointer, new_size);
    
    if (UTEST_NULL == new_pointer) {
        free(new_pointer);
    }
    
    return new_pointer;
}

static UTEST_INLINE utest_int64_t utest_ns(void) {
#if defined(_MSC_VER) || defined(__MINGW64__) || defined(__MINGW32__)
    utest_large_integer counter;
    utest_large_integer frequency;
    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&frequency);
    return UTEST_CAST(utest_int64_t,
                      (counter.QuadPart * 1000000000) / frequency.QuadPart);
#elif defined(__linux__) && defined(__STRICT_ANSI__)
    return UTEST_CAST(utest_int64_t, clock()) * 1000000000 / CLOCKS_PER_SEC;
#elif defined(__linux__) \
|| defined(__FreeBSD__) \
|| defined(__OpenBSD__) \
|| defined(__NetBSD__) \
|| defined(__DragonFly__) \
|| defined(__sun__) \
|| defined(__HAIKU__)
    struct timespec ts;
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && !defined(__HAIKU__)
    timespec_get(&ts, TIME_UTC);
#else
    const clockid_t cid = CLOCK_REALTIME;
#if defined(UTEST_USE_CLOCKGETTIME)
    clock_gettime(cid, &ts);
#else
    syscall(SYS_clock_gettime, cid, &ts);
#endif
#endif
    return UTEST_CAST(utest_int64_t, ts.tv_sec) * 1000 * 1000 * 1000 + ts.tv_nsec;
#elif __APPLE__
    return UTEST_CAST(utest_int64_t, mach_absolute_time());
#elif __EMSCRIPTEN__
    return emscripten_performance_now() * 1000000.0;
#else
#error Unsupported platform!
#endif
}

typedef void (*utest_testcase_t)(int *, size_t);

struct utest_test_state_s {
    utest_testcase_t func;
    size_t index;
    char *name;
};

struct utest_state_s {
    struct utest_test_state_s *tests;
    size_t tests_length;
    FILE *output;
};

/* extern to the global state utest needs to execute */
UTEST_EXTERN struct utest_state_s utest_state;

#if defined(_MSC_VER)
#define UTEST_WEAK __forceinline
#else
#define UTEST_WEAK __attribute__((weak))
#endif

#if defined(_MSC_VER)
#define UTEST_UNUSED
#else
#define UTEST_UNUSED __attribute__((unused))
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvariadic-macros"
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#endif
#define UTEST_PRINTF(...)                                                      \
if (utest_state.output) {                                                    \
fprintf(utest_state.output, __VA_ARGS__);                                  \
}                                                                            \
printf(__VA_ARGS__)
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvariadic-macros"
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#endif

#ifdef _MSC_VER
#define UTEST_SNPRINTF(BUFFER, N, ...) _snprintf_s(BUFFER, N, N, __VA_ARGS__)
#else
#define UTEST_SNPRINTF(...) snprintf(__VA_ARGS__)
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#if defined(__cplusplus)
/* if we are using c++ we can use overloaded methods (its in the language) */
#define UTEST_OVERLOADABLE
#elif defined(__clang__)
/* otherwise, if we are using clang with c - use the overloadable attribute */
#define UTEST_OVERLOADABLE __attribute__((overloadable))
#endif

#if defined(UTEST_OVERLOADABLE)
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(float f);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(float f) {
    UTEST_PRINTF("%f", UTEST_CAST(double, f));
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(double d);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(double d) {
    UTEST_PRINTF("%f", d);
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long double d);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long double d) {
    UTEST_PRINTF("%Lf", d);
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(int i) {
    UTEST_PRINTF("%d", i);
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(unsigned int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(unsigned int i) {
    UTEST_PRINTF("%u", i);
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long int i) {
    UTEST_PRINTF("%ld", i);
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long unsigned int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long unsigned int i) {
    UTEST_PRINTF("%lu", i);
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(const void *p);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(const void *p) {
    UTEST_PRINTF("%p", p);
}

/*
   long long is a c++11 extension
*/
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) ||              \
defined(__cplusplus) && (__cplusplus >= 201103L)

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#endif

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long long int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long long int i) {
    UTEST_PRINTF("%lld", i);
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long long unsigned int i);
UTEST_WEAK UTEST_OVERLOADABLE void
utest_type_printer(long long unsigned int i) {
    UTEST_PRINTF("%llu", i);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#define utest_type_printer(val)                                                \
UTEST_PRINTF(_Generic((val), signed char                                     \
: "%d", unsigned char                                  \
: "%u", short                                          \
: "%d", unsigned short                                 \
: "%u", int                                            \
: "%d", long                                           \
: "%ld", long long                                     \
: "%lld", unsigned                                     \
: "%u", unsigned long                                  \
: "%lu", unsigned long long                            \
: "%llu", float                                        \
: "%f", double                                         \
: "%f", long double                                    \
: "%Lf", default                                       \
: _Generic((val - val), ptrdiff_t                      \
: "%p", default                             \
: "undef")),                                \
(val))
#else
/*
   we don't have the ability to print the values we got, so we create a macro
   to tell our users we can't do anything fancy
*/
#define utest_type_printer(...) UTEST_PRINTF("undef")
#endif

#ifdef _MSC_VER
#define UTEST_SURPRESS_WARNING_BEGIN                                           \
__pragma(warning(push)) __pragma(warning(disable : 4127))
#define UTEST_SURPRESS_WARNING_END __pragma(warning(pop))
#else
#define UTEST_SURPRESS_WARNING_BEGIN
#define UTEST_SURPRESS_WARNING_END
#endif

#if defined(__cplusplus) && (__cplusplus >= 201103L)
#define UTEST_AUTO(x) auto
#elif !defined(__cplusplus)

#if defined(__clang__)
/* clang-format off */
/* had to disable clang-format here because it malforms the pragmas */
#define UTEST_AUTO(x)                                                          \
_Pragma("clang diagnostic push")                                             \
_Pragma("clang diagnostic ignored \"-Wgnu-auto-type\"") __auto_type      \
_Pragma("clang diagnostic pop")
/* clang-format on */
#else
#define UTEST_AUTO(x) __typeof__(x + 0)
#endif

#else
#define UTEST_AUTO(x) typeof(x + 0)
#endif

#if defined(__clang__)
#define UTEST_STRNCMP(x, y, size)                                              \
_Pragma("clang diagnostic push")                                             \
_Pragma("clang diagnostic ignored \"-Wdisabled-macro-expansion\"")       \
strncmp(x, y, size) _Pragma("clang diagnostic pop")
#else
#define UTEST_STRNCMP(x, y, size) strncmp(x, y, size)
#endif

#if defined(__clang__)
#define UTEST_EXPECT(x, y, cond)                                               \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
_Pragma("clang diagnostic push")                                           \
_Pragma("clang diagnostic ignored \"-Wlanguage-extension-token\"")     \
_Pragma("clang diagnostic ignored \"-Wc++98-compat-pedantic\"")    \
_Pragma("clang diagnostic ignored \"-Wfloat-equal\"")          \
UTEST_AUTO(x) xEval = (x);                                 \
UTEST_AUTO(y) yEval = (y);                                                 \
if (!((xEval)cond(yEval))) {                                               \
_Pragma("clang diagnostic pop")                                          \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                \
UTEST_PRINTF("  Expected : ");                                           \
utest_type_printer(xEval);                                               \
UTEST_PRINTF("\n");                                                      \
UTEST_PRINTF("    Actual : ");                                           \
utest_type_printer(yEval);                                               \
UTEST_PRINTF("\n");                                                      \
*utest_result = 1;                                                       \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END
#elif defined(__GNUC__)
#define UTEST_EXPECT(x, y, cond)                                               \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
UTEST_AUTO(x) xEval = (x);                                                 \
UTEST_AUTO(y) yEval = (y);                                                 \
if (!((xEval)cond(yEval))) {                                               \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : ");                                           \
utest_type_printer(xEval);                                               \
UTEST_PRINTF("\n");                                                      \
UTEST_PRINTF("    Actual : ");                                           \
utest_type_printer(yEval);                                               \
UTEST_PRINTF("\n");                                                      \
*utest_result = 1;                                                       \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END
#else
#define UTEST_EXPECT(x, y, cond)                                               \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (!((x)cond(y))) {                                                       \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
*utest_result = 1;                                                       \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END
#endif

#define EXPECT_TRUE(x)                                                         \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (!(x)) {                                                                \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : true\n");                                     \
UTEST_PRINTF("    Actual : %s\n", (x) ? "true" : "false");               \
*utest_result = 1;                                                       \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END

#define EXPECT_FALSE(x)                                                        \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (x) {                                                                   \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : false\n");                                    \
UTEST_PRINTF("    Actual : %s\n", (x) ? "true" : "false");               \
*utest_result = 1;                                                       \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END

#define EXPECT_EQ(x, y) UTEST_EXPECT(x, y, ==)
#define EXPECT_NE(x, y) UTEST_EXPECT(x, y, !=)
#define EXPECT_LT(x, y) UTEST_EXPECT(x, y, <)
#define EXPECT_LE(x, y) UTEST_EXPECT(x, y, <=)
#define EXPECT_GT(x, y) UTEST_EXPECT(x, y, >)
#define EXPECT_GE(x, y) UTEST_EXPECT(x, y, >=)

#define EXPECT_STREQ(x, y)                                                     \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (0 != strcmp(x, y)) {                                                   \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : \"%s\"\n", x);                                \
UTEST_PRINTF("    Actual : \"%s\"\n", y);                                \
*utest_result = 1;                                                       \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END

#define EXPECT_STRNE(x, y)                                                     \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (0 == strcmp(x, y)) {                                                   \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : \"%s\"\n", x);                                \
UTEST_PRINTF("    Actual : \"%s\"\n", y);                                \
*utest_result = 1;                                                       \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END

#define EXPECT_STRNEQ(x, y, n)                                                 \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (0 != UTEST_STRNCMP(x, y, n)) {                                         \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : \"%.*s\"\n", UTEST_CAST(int, n), x);          \
UTEST_PRINTF("    Actual : \"%.*s\"\n", UTEST_CAST(int, n), y);          \
*utest_result = 1;                                                       \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END

#define EXPECT_STRNNE(x, y, n)                                                 \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (0 == UTEST_STRNCMP(x, y, n)) {                                         \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : \"%.*s\"\n", UTEST_CAST(int, n), x);          \
UTEST_PRINTF("    Actual : \"%.*s\"\n", UTEST_CAST(int, n), y);          \
*utest_result = 1;                                                       \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END

#define EXPECT_NEAR(x, y, epsilon)                                             \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (utest_fabs(UTEST_CAST(double, x) - UTEST_CAST(double, y)) >            \
UTEST_CAST(double, epsilon)) {                                         \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : %f\n", UTEST_CAST(double, x));                \
UTEST_PRINTF("    Actual : %f\n", UTEST_CAST(double, y));                \
*utest_result = 1;                                                       \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END

#if defined(__clang__)
#define UTEST_ASSERT(x, y, cond)                                               \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
_Pragma("clang diagnostic push")                                           \
_Pragma("clang diagnostic ignored \"-Wlanguage-extension-token\"")     \
_Pragma("clang diagnostic ignored \"-Wc++98-compat-pedantic\"")    \
_Pragma("clang diagnostic ignored \"-Wfloat-equal\"")          \
UTEST_AUTO(x) xEval = (x);                                 \
UTEST_AUTO(y) yEval = (y);                                                 \
if (!((xEval)cond(yEval))) {                                               \
_Pragma("clang diagnostic pop")                                          \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                \
UTEST_PRINTF("  Expected : ");                                           \
utest_type_printer(xEval);                                               \
UTEST_PRINTF("\n");                                                      \
UTEST_PRINTF("    Actual : ");                                           \
utest_type_printer(yEval);                                               \
UTEST_PRINTF("\n");                                                      \
*utest_result = 1;                                                       \
return;                                                                  \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END
#elif defined(__GNUC__)
#define UTEST_ASSERT(x, y, cond)                                               \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
UTEST_AUTO(x) xEval = (x);                                                 \
UTEST_AUTO(y) yEval = (y);                                                 \
if (!((xEval)cond(yEval))) {                                               \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : ");                                           \
utest_type_printer(xEval);                                               \
UTEST_PRINTF("\n");                                                      \
UTEST_PRINTF("    Actual : ");                                           \
utest_type_printer(yEval);                                               \
UTEST_PRINTF("\n");                                                      \
*utest_result = 1;                                                       \
return;                                                                  \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END
#else
#define UTEST_ASSERT(x, y, cond)                                               \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (!((x)cond(y))) {                                                       \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
*utest_result = 1;                                                       \
return;                                                                  \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END
#endif

#define ASSERT_TRUE(x)                                                         \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (!(x)) {                                                                \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : true\n");                                     \
UTEST_PRINTF("    Actual : %s\n", (x) ? "true" : "false");               \
*utest_result = 1;                                                       \
return;                                                                  \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END

#define ASSERT_FALSE(x)                                                        \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (x) {                                                                   \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : false\n");                                    \
UTEST_PRINTF("    Actual : %s\n", (x) ? "true" : "false");               \
*utest_result = 1;                                                       \
return;                                                                  \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END

#define ASSERT_EQ(x, y) UTEST_ASSERT(x, y, ==)
#define ASSERT_NE(x, y) UTEST_ASSERT(x, y, !=)
#define ASSERT_LT(x, y) UTEST_ASSERT(x, y, <)
#define ASSERT_LE(x, y) UTEST_ASSERT(x, y, <=)
#define ASSERT_GT(x, y) UTEST_ASSERT(x, y, >)
#define ASSERT_GE(x, y) UTEST_ASSERT(x, y, >=)

#define ASSERT_STREQ(x, y)                                                     \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (0 != strcmp(x, y)) {                                                   \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : \"%s\"\n", x);                                \
UTEST_PRINTF("    Actual : \"%s\"\n", y);                                \
*utest_result = 1;                                                       \
return;                                                                  \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END

#define ASSERT_STRNE(x, y)                                                     \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (0 == strcmp(x, y)) {                                                   \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : \"%s\"\n", x);                                \
UTEST_PRINTF("    Actual : \"%s\"\n", y);                                \
*utest_result = 1;                                                       \
return;                                                                  \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END

#define ASSERT_STRNEQ(x, y, n)                                                 \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (0 != UTEST_STRNCMP(x, y, n)) {                                         \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : \"%.*s\"\n", UTEST_CAST(int, n), x);          \
UTEST_PRINTF("    Actual : \"%.*s\"\n", UTEST_CAST(int, n), y);          \
*utest_result = 1;                                                       \
return;                                                                  \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END

#define ASSERT_STRNNE(x, y, n)                                                 \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (0 == UTEST_STRNCMP(x, y, n)) {                                         \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : \"%.*s\"\n", UTEST_CAST(int, n), x);          \
UTEST_PRINTF("    Actual : \"%.*s\"\n", UTEST_CAST(int, n), y);          \
*utest_result = 1;                                                       \
return;                                                                  \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END

#define ASSERT_NEAR(x, y, epsilon)                                             \
UTEST_SURPRESS_WARNING_BEGIN do {                                            \
if (utest_fabs(UTEST_CAST(double, x) - UTEST_CAST(double, y)) >            \
UTEST_CAST(double, epsilon)) {                                         \
UTEST_PRINTF("%s:%u: Failure\n", __FILE__, __LINE__);                    \
UTEST_PRINTF("  Expected : %f\n", UTEST_CAST(double, x));                \
UTEST_PRINTF("    Actual : %f\n", UTEST_CAST(double, y));                \
*utest_result = 1;                                                       \
return;                                                                  \
}                                                                          \
}                                                                            \
while (0)                                                                    \
UTEST_SURPRESS_WARNING_END

#define UTEST(SET, NAME)                                                       \
UTEST_EXTERN struct utest_state_s utest_state;                               \
static void utest_run_##SET##_##NAME(int *utest_result);                     \
static void utest_##SET##_##NAME(int *utest_result, size_t utest_index) {    \
(void)utest_index;                                                         \
utest_run_##SET##_##NAME(utest_result);                                    \
}                                                                            \
UTEST_INITIALIZER(utest_register_##SET##_##NAME) {                           \
const size_t index = utest_state.tests_length++;                           \
const char *name_part = #SET "." #NAME;                                    \
const size_t name_size = strlen(name_part) + 1;                            \
char *name = UTEST_PTR_CAST(char *, malloc(name_size));                    \
utest_state.tests = UTEST_PTR_CAST(                                        \
struct utest_test_state_s *,                                           \
utest_realloc(UTEST_PTR_CAST(void *, utest_state.tests),               \
sizeof(struct utest_test_state_s) *                      \
utest_state.tests_length));                          \
if (utest_state.tests) {                                                   \
utest_state.tests[index].func = &utest_##SET##_##NAME;                   \
utest_state.tests[index].name = name;                                    \
utest_state.tests[index].index = 0;                                      \
}                                                                          \
UTEST_SNPRINTF(name, name_size, "%s", name_part);                          \
}                                                                            \
void utest_run_##SET##_##NAME(int *utest_result)

#define UTEST_F_SETUP(FIXTURE)                                                 \
static void utest_f_setup_##FIXTURE(int *utest_result,                       \
struct FIXTURE *utest_fixture)

#define UTEST_F_TEARDOWN(FIXTURE)                                              \
static void utest_f_teardown_##FIXTURE(int *utest_result,                    \
struct FIXTURE *utest_fixture)

#define UTEST_F(FIXTURE, NAME)                                                 \
UTEST_EXTERN struct utest_state_s utest_state;                               \
static void utest_f_setup_##FIXTURE(int *, struct FIXTURE *);                \
static void utest_f_teardown_##FIXTURE(int *, struct FIXTURE *);             \
static void utest_run_##FIXTURE##_##NAME(int *, struct FIXTURE *);           \
static void utest_f_##FIXTURE##_##NAME(int *utest_result,                    \
size_t utest_index) {                 \
struct FIXTURE fixture;                                                    \
(void)utest_index;                                                         \
memset(&fixture, 0, sizeof(fixture));                                      \
utest_f_setup_##FIXTURE(utest_result, &fixture);                           \
if (0 != *utest_result) {                                                  \
return;                                                                  \
}                                                                          \
utest_run_##FIXTURE##_##NAME(utest_result, &fixture);                      \
utest_f_teardown_##FIXTURE(utest_result, &fixture);                        \
}                                                                            \
UTEST_INITIALIZER(utest_register_##FIXTURE##_##NAME) {                       \
const size_t index = utest_state.tests_length++;                           \
const char *name_part = #FIXTURE "." #NAME;                                \
const size_t name_size = strlen(name_part) + 1;                            \
char *name = UTEST_PTR_CAST(char *, malloc(name_size));                    \
utest_state.tests = UTEST_PTR_CAST(                                        \
struct utest_test_state_s *,                                           \
utest_realloc(UTEST_PTR_CAST(void *, utest_state.tests),               \
sizeof(struct utest_test_state_s) *                      \
utest_state.tests_length));                          \
utest_state.tests[index].func = &utest_f_##FIXTURE##_##NAME;               \
utest_state.tests[index].name = name;                                      \
UTEST_SNPRINTF(name, name_size, "%s", name_part);                          \
}                                                                            \
void utest_run_##FIXTURE##_##NAME(int *utest_result,                         \
struct FIXTURE *utest_fixture)

#define UTEST_I_SETUP(FIXTURE)                                                 \
static void utest_i_setup_##FIXTURE(                                         \
int *utest_result, struct FIXTURE *utest_fixture, size_t utest_index)

#define UTEST_I_TEARDOWN(FIXTURE)                                              \
static void utest_i_teardown_##FIXTURE(                                      \
int *utest_result, struct FIXTURE *utest_fixture, size_t utest_index)

#define UTEST_I(FIXTURE, NAME, INDEX)                                          \
UTEST_EXTERN struct utest_state_s utest_state;                               \
static void utest_run_##FIXTURE##_##NAME##_##INDEX(int *, struct FIXTURE *); \
static void utest_i_##FIXTURE##_##NAME##_##INDEX(int *utest_result,          \
size_t index) {             \
struct FIXTURE fixture;                                                    \
memset(&fixture, 0, sizeof(fixture));                                      \
utest_i_setup_##FIXTURE(utest_result, &fixture, index);                    \
if (0 != *utest_result) {                                                  \
return;                                                                  \
}                                                                          \
utest_run_##FIXTURE##_##NAME##_##INDEX(utest_result, &fixture);            \
utest_i_teardown_##FIXTURE(utest_result, &fixture, index);                 \
}                                                                            \
UTEST_INITIALIZER(utest_register_##FIXTURE##_##NAME##_##INDEX) {             \
size_t i;                                                                  \
utest_uint64_t iUp;                                                        \
for (i = 0; i < (INDEX); i++) {                                            \
const size_t index = utest_state.tests_length++;                         \
const char *name_part = #FIXTURE "." #NAME;                              \
const size_t name_size = strlen(name_part) + 32;                         \
char *name = UTEST_PTR_CAST(char *, malloc(name_size));                  \
utest_state.tests = UTEST_PTR_CAST(                                      \
struct utest_test_state_s *,                                         \
utest_realloc(UTEST_PTR_CAST(void *, utest_state.tests),             \
sizeof(struct utest_test_state_s) *                    \
utest_state.tests_length));                        \
utest_state.tests[index].func = &utest_i_##FIXTURE##_##NAME##_##INDEX;   \
utest_state.tests[index].index = i;                                      \
utest_state.tests[index].name = name;                                    \
iUp = UTEST_CAST(utest_uint64_t, i);                                     \
UTEST_SNPRINTF(name, name_size, "%s/%" UTEST_PRIu64, name_part, iUp);    \
}                                                                          \
}                                                                            \
void utest_run_##FIXTURE##_##NAME##_##INDEX(int *utest_result,               \
struct FIXTURE *utest_fixture)

UTEST_WEAK
double utest_fabs(double d);
UTEST_WEAK
double utest_fabs(double d) {
    union {
        double d;
        utest_uint64_t u;
    } both;
    both.d = d;
    both.u &= 0x7fffffffffffffffu;
    return both.d;
}

UTEST_WEAK
int utest_should_filter_test(const char *filter, const char *testcase);
UTEST_WEAK int utest_should_filter_test(const char *filter,
                                        const char *testcase) {
    if (filter) {
        const char *filter_cur = filter;
        const char *testcase_cur = testcase;
        const char *filter_wildcard = UTEST_NULL;
        
        while (('\0' != *filter_cur) && ('\0' != *testcase_cur)) {
            if ('*' == *filter_cur) {
                /* store the position of the wildcard */
                filter_wildcard = filter_cur;
                
                /* skip the wildcard character */
                filter_cur++;
                
                while (('\0' != *filter_cur) && ('\0' != *testcase_cur)) {
                    if ('*' == *filter_cur) {
                        /*
                           we found another wildcard (filter is something like *foo*) so we
                           exit the current loop, and return to the parent loop to handle
                           the wildcard case
                        */
                        break;
                    } else if (*filter_cur != *testcase_cur) {
                        /* otherwise our filter didn't match, so reset it */
                        filter_cur = filter_wildcard;
                    }
                    
                    /* move testcase along */
                    testcase_cur++;
                    
                    /* move filter along */
                    filter_cur++;
                }
                
                if (('\0' == *filter_cur) && ('\0' == *testcase_cur)) {
                    return 0;
                }
                
                /* if the testcase has been exhausted, we don't have a match! */
                if ('\0' == *testcase_cur) {
                    return 1;
                }
            } else {
                if (*testcase_cur != *filter_cur) {
                    /* test case doesn't match filter */
                    return 1;
                } else {
                    /* move our filter and testcase forward */
                    testcase_cur++;
                    filter_cur++;
                }
            }
        }
        
        if (('\0' != *filter_cur) ||
            (('\0' != *testcase_cur) &&
             ((filter == filter_cur) || ('*' != filter_cur[-1])))) {
            /* we have a mismatch! */
            return 1;
        }
    }
    
    return 0;
}

static UTEST_INLINE FILE *utest_fopen(const char *filename, const char *mode) {
#ifdef _MSC_VER
    FILE *file;
    if (0 == fopen_s(&file, filename, mode)) {
        return file;
    } else {
        return UTEST_NULL;
    }
#else
    return fopen(filename, mode);
#endif
}

static UTEST_INLINE int utest_main(int argc, const char *const argv[]);
int utest_main(int argc, const char *const argv[]) {
    utest_uint64_t failed = 0;
    size_t index = 0;
    size_t *failed_testcases = UTEST_NULL;
    size_t failed_testcases_length = 0;
    const char *filter = UTEST_NULL;
    utest_uint64_t ran_tests = 0;
    int enable_mixed_units = 0;
    int random_order = 0;
    utest_uint32_t seed = 0;
    
    enum colours { RESET, GREEN, RED };
    
    const int use_colours = UTEST_COLOUR_OUTPUT();
    const char *colours[] = {"\033[0m", "\033[32m", "\033[31m"};
    
    if (!use_colours) {
        for (index = 0; index < sizeof colours / sizeof colours[0]; index++) {
            colours[index] = "";
        }
    }
    /* loop through all arguments looking for our options */
    for (index = 1; index < UTEST_CAST(size_t, argc); index++) {
        /* Informational switches */
        const char help_str[] = "--help";
        const char list_str[] = "--list-tests";
        /* Test config switches */
        const char filter_str[] = "--filter=";
        const char output_str[] = "--output=";
        const char enable_mixed_units_str[] = "--enable-mixed-units";
        const char random_order_str[] = "--random-order";
        const char random_order_with_seed_str[] = "--random-order=";
        
        if (0 == UTEST_STRNCMP(argv[index], help_str, strlen(help_str))) {
            printf("utest.h - the single file unit testing solution for C/C++!\n"
                   "Command line Options:\n"
                   "  --help                  Show this message and exit.\n"
                   "  --filter=<filter>       Filter the test cases to run (EG. "
                   "MyTest*.a would run MyTestCase.a but not MyTestCase.b).\n"
                   "  --list-tests            List testnames, one per line. Output "
                   "names can be passed to --filter.\n");
            printf("  --output=<output>       Output an xunit XML file to the file "
                   "specified in <output>.\n"
                   "  --enable-mixed-units    Enable the per-test output to contain "
                   "mixed units (s/ms/us/ns).\n"
                   "  --random-order[=<seed>] Randomize the order that the tests are "
                   "ran in. If the optional <seed> argument is not provided, then a "
                   "random starting seed is used.\n");
            goto cleanup;
        } else if (0 ==
                   UTEST_STRNCMP(argv[index], filter_str, strlen(filter_str))) {
            /* user wants to filter what test cases run! */
            filter = argv[index] + strlen(filter_str);
        } else if (0 ==
                   UTEST_STRNCMP(argv[index], output_str, strlen(output_str))) {
            utest_state.output = utest_fopen(argv[index] + strlen(output_str), "w+");
        } else if (0 == UTEST_STRNCMP(argv[index], list_str, strlen(list_str))) {
            for (index = 0; index < utest_state.tests_length; index++) {
                UTEST_PRINTF("%s\n", utest_state.tests[index].name);
            }
            /* when printing the test list, don't actually run the tests */
            return 0;
        } else if (0 == UTEST_STRNCMP(argv[index], enable_mixed_units_str,
                                      strlen(enable_mixed_units_str))) {
            enable_mixed_units = 1;
        } else if (0 == UTEST_STRNCMP(argv[index], random_order_with_seed_str,
                                      strlen(random_order_with_seed_str))) {
            seed =
                UTEST_CAST(utest_uint32_t,
                           strtoul(argv[index] + strlen(random_order_with_seed_str),
                                   UTEST_NULL, 10));
            random_order = 1;
        } else if (0 == UTEST_STRNCMP(argv[index], random_order_str,
                                      strlen(random_order_str))) {
            const utest_int64_t ns = utest_ns();
            
            // Some really poor pseudo-random using the current time. I do this
            // because I really want to avoid using C's rand() because that'd mean our
            // random would be affected by any srand() usage by the user (which I
            // don't want).
            seed = UTEST_CAST(utest_uint32_t, ns >> 32) * 31 +
                UTEST_CAST(utest_uint32_t, ns & 0xffffffff);
            random_order = 1;
        }
    }
    
    if (random_order) {
        // Use Fisher-Yates with the Durstenfield's version to randomly re-order the
        // tests.
        for (index = utest_state.tests_length; index > 1; index--) {
            // For the random order we'll use PCG.
            const utest_uint32_t state = seed;
            const utest_uint32_t word =
            ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
            const utest_uint32_t next = ((word >> 22u) ^ word) % index;
            
            // Swap the randomly chosen element into the last location.
            const struct utest_test_state_s copy = utest_state.tests[index - 1];
            utest_state.tests[index - 1] = utest_state.tests[next];
            utest_state.tests[next] = copy;
            
            // Move the seed onwards.
            seed = seed * 747796405u + 2891336453u;
        }
    }
    
    for (index = 0; index < utest_state.tests_length; index++) {
        if (utest_should_filter_test(filter, utest_state.tests[index].name)) {
            continue;
        }
        
        ran_tests++;
    }
    
    printf("%s[==========]%s Running %" UTEST_PRIu64 " test cases.\n",
           colours[GREEN], colours[RESET], UTEST_CAST(utest_uint64_t, ran_tests));
    
    if (utest_state.output) {
        fprintf(utest_state.output, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        fprintf(utest_state.output,
                "<testsuites tests=\"%" UTEST_PRIu64 "\" name=\"All\">\n",
                UTEST_CAST(utest_uint64_t, ran_tests));
        fprintf(utest_state.output,
                "<testsuite name=\"Tests\" tests=\"%" UTEST_PRIu64 "\">\n",
                UTEST_CAST(utest_uint64_t, ran_tests));
    }
    
    for (index = 0; index < utest_state.tests_length; index++) {
        int result = 0;
        utest_int64_t ns = 0;
        
        if (utest_should_filter_test(filter, utest_state.tests[index].name)) {
            continue;
        }
        
        printf("%s[ RUN      ]%s %s\n", colours[GREEN], colours[RESET],
               utest_state.tests[index].name);
        
        if (utest_state.output) {
            fprintf(utest_state.output, "<testcase name=\"%s\">",
                    utest_state.tests[index].name);
        }
        
        ns = utest_ns();
        errno = 0;
        utest_state.tests[index].func(&result, utest_state.tests[index].index);
        ns = utest_ns() - ns;
        
        if (utest_state.output) {
            fprintf(utest_state.output, "</testcase>\n");
        }
        
        // Record the failing test.
        if (0 != result) {
            const size_t failed_testcase_index = failed_testcases_length++;
            failed_testcases = UTEST_PTR_CAST(
                                              size_t *, utest_realloc(UTEST_PTR_CAST(void *, failed_testcases),
                                                                      sizeof(size_t) * failed_testcases_length));
            if (UTEST_NULL != failed_testcases) {
                failed_testcases[failed_testcase_index] = index;
            }
            failed++;
        }
        
        {
            const char *const units[] = {"ns", "us", "ms", "s", UTEST_NULL};
            unsigned int unit_index = 0;
            utest_int64_t time = ns;
            
            if (enable_mixed_units) {
                for (unit_index = 0; UTEST_NULL != units[unit_index]; unit_index++) {
                    if (10000 > time) {
                        break;
                    }
                    
                    time /= 1000;
                }
            }
            
            if (0 != result) {
                printf("%s[  FAILED  ]%s %s (%" UTEST_PRId64 "%s)\n", colours[RED],
                       colours[RESET], utest_state.tests[index].name, time,
                       units[unit_index]);
            } else {
                printf("%s[       OK ]%s %s (%" UTEST_PRId64 "%s)\n", colours[GREEN],
                       colours[RESET], utest_state.tests[index].name, time,
                       units[unit_index]);
            }
        }
    }
    
    printf("%s[==========]%s %" UTEST_PRIu64 " test cases ran.\n", colours[GREEN],
           colours[RESET], ran_tests);
    printf("%s[  PASSED  ]%s %" UTEST_PRIu64 " tests.\n", colours[GREEN],
           colours[RESET], ran_tests - failed);
    
    if (0 != failed) {
        printf("%s[  FAILED  ]%s %" UTEST_PRIu64 " tests, listed below:\n",
               colours[RED], colours[RESET], failed);
        for (index = 0; index < failed_testcases_length; index++) {
            printf("%s[  FAILED  ]%s %s\n", colours[RED], colours[RESET],
                   utest_state.tests[failed_testcases[index]].name);
        }
    }
    
    if (utest_state.output) {
        fprintf(utest_state.output, "</testsuite>\n</testsuites>\n");
    }
    
    cleanup:
    for (index = 0; index < utest_state.tests_length; index++) {
        free(UTEST_PTR_CAST(void *, utest_state.tests[index].name));
    }
    
    free(UTEST_PTR_CAST(void *, failed_testcases));
    free(UTEST_PTR_CAST(void *, utest_state.tests));
    
    if (utest_state.output) {
        fclose(utest_state.output);
    }
    
    return UTEST_CAST(int, failed);
}

/*
   we need, in exactly one source file, define the global struct that will hold
   the data we need to run utest. This macro allows the user to declare the
   data without having to use the UTEST_MAIN macro, thus allowing them to write
   their own main() function.
*/
#define UTEST_STATE() struct utest_state_s utest_state = {0, 0, 0}

/*
   define a main() function to call into utest.h and start executing tests! A
   user can optionally not use this macro, and instead define their own main()
   function and manually call utest_main. The user must, in exactly one source
   file, use the UTEST_STATE macro to declare a global struct variable that
   utest requires.
*/
#define UTEST_MAIN()                                                           \
UTEST_STATE();                                                               \
int main(int argc, const char *const argv[]) {                               \
return utest_main(argc, argv);                                             \
}

#endif /* SHEREDOM_UTEST_H_INCLUDED */

#if 1
UTEST(ak_bucket_array, Tests)
{
    ak_bucket_array<int32_t, 3> Array;
    
    Array.Add(5);
    
    int32_t T = *Array.Pop();
    
    ASSERT_EQ(Array.CurrentBucketIndex, 0);
    ASSERT_EQ(Array.Length, 0);
    ASSERT_EQ(Array.Buckets[0]->Length, 0);
    ASSERT_EQ(T, 5);
    
    Array.Add(6);
    Array.Add(12);
    
    ASSERT_EQ(Array.Length, 2);
    ASSERT_EQ(Array.Buckets.Length, 1);
    ASSERT_EQ(Array[1], 12);
    
    Array.Add(4);
    Array.Add(5);
    ASSERT_EQ(Array.CurrentBucketIndex, 1);
    ASSERT_EQ(Array.Buckets[1]->Length, 1);
    
    T = *Array.Pop();
    ASSERT_EQ(Array.CurrentBucketIndex, 0);
    ASSERT_EQ(Array.Buckets[0]->Length, 3);
    ASSERT_EQ(T, 5);
    
    Array.Add(5);
    
    int32_t Temp[5] = {1, 2, 3, 4, 5};
    Array.Add_Range(Temp, 5);
    
    ASSERT_EQ(Array.CurrentBucketIndex, 2);
    ASSERT_EQ(Array.Buckets[2]->Length, 3);
    
    ASSERT_EQ(Array.Length, 9);
    ASSERT_EQ(Array[7], 4);
    
    
    ak_bucket_array<int32_t, 3> Array2 = Array.Copy();
    ASSERT_EQ(Array.Length, Array2.Length);
    ASSERT_EQ(Array.CurrentBucketIndex, Array2.CurrentBucketIndex);
    ASSERT_EQ(Array.Buckets.Length, Array2.Buckets.Length);
    
    for(uint64_t Index = 0; Index < Array.Length; Index++)
    {
        ASSERT_EQ(Array[Index], Array2[Index]);
    }
    
    uint32_t Length = 0;
    for(int32_t x : Array)
    {
        Length++;
    }
    ASSERT_EQ(Length, Array.Length);
    
    for(uint64_t BucketIndex = 0; BucketIndex < Array.Buckets.Length; BucketIndex++)
    {
        ASSERT_EQ(Array.Buckets[BucketIndex]->Length, Array2.Buckets[BucketIndex]->Length);
    }
    
    ak_bucket_array<int32_t, 3> Array3 = Array.Copy();
    
    Array.Resize(7);
    Array2.Resize(17);
    Array3.Resize(2);
    
    ASSERT_EQ(Array.CurrentBucketIndex, 2);
    ASSERT_EQ(Array2.Buckets.Length, 6);
    ASSERT_EQ(Array2.CurrentBucketIndex, 5);
    ASSERT_EQ(Array3.CurrentBucketIndex, 0);
}

UTEST(ak_hashmap, Tests)
{
    ak_hashmap<uint32_t, uint32_t> Map;
    
    Map.Add(5, 10);
    Map.Add(10, 13);
    Map.Add(4, 2);
    
    uint32_t* A = Map.Find(10);
    ASSERT_EQ(*A, 13);
    
    uint32_t* B = Map.Find(5);
    ASSERT_EQ(*B, 10);
    
    uint32_t* C = Map.Find(8);
    ASSERT_EQ(C, NULL);
    
    ASSERT_EQ(Map.Length, 3);
    
    uint32_t Length = 0;
    for(auto pair : Map)
    {
        Length++;
    }
    ASSERT_EQ(Length, 3);
    
    Map.Remove(10);
    ASSERT_EQ(Map.Length, 2);
    
    uint32_t* D = Map.Find(10);
    ASSERT_EQ(D, NULL);
}

UTEST(ak_pool, Tests)
{
    ak_pool<uint32_t> P;
    
    uint64_t ID;
    uint32_t* Data = P.Allocate_And_Get(&ID);
    
    uint64_t ID2 = P.Allocate(5);
    
    uint64_t ID3 = P.Allocate(2);
    
    uint64_t ID4 = P.Allocate(6);
    
    P.Free(ID2);
    P.Free(ID4);
    
    P.Allocate(8);
    uint64_t ID5 = P.Allocate(9);
    
    for(uint32_t& x : P)
    {
        x = 2;
    }
    
    for(const uint32_t& x : P)
    { 
        ASSERT_EQ(x, 2);
    }
}



#endif

UTEST_MAIN();

#endif // AK_STD_TESTS