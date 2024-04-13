/***************************************************************
 *
 * file: pss.h
 *
 * @Author  Nikolaos Vasilikopoulos (nvasilik@csd.uoc.gr), John Petropoulos (johnpetr@csd.uoc.gr)
 * @Version 30-11-2022
 *
 * @e-mail       hy240-list@csd.uoc.gr
 *
 * @brief   Implementation of the "pss.h" header file for the Public Subscribe System,
 * function definitions
 *
 *
 ***************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "pss.h"

struct Group G[MG];
struct SubInfo *HT[MG];
static int P;
static int M;
static int A;
static int B;

bool Info_isUnique_iId(int id, int tm);
bool Subscriber_isUnique_sId(int id);
Sub* Subscriber_Insert(Sub* List, int id);
Sub* Subscriber_Delete(Sub* List, int id);
SubInfo *SubInfo_Insert(SubInfo *List, int tm, int id, int *gids_arr, int size_of_gids_arr);
bool SubInfo_Interested(const int* gids_arr, int size_of_gids_arr, int k);
SubInfo *SubInfo_Delete(SubInfo *List, int id);
TreeInfo* ConsumeInfo(TreeInfo *info, TreeInfo *end);
void Insert_Info_Print(int iTM,int iId, const int *gids_arr, int size_of_gids_arr);
void Delete_Subscriber_Print(int sId, Info **gids_arr);
void Subscriber_Registration_Print(int sTM, int sId, int *gids_arr, int size_of_gids_arr);
void Consume_Print(SubInfo* sub, TreeInfo** preConsume);
void Consume_Print_Info(TreeInfo *info, TreeInfo *end);
SubInfo *getSub(int id);
void filterArray(int *gids_arr, int *size_of_gids_arr);
bool isSubValid(int sId);
void Hash_Insert(int sTM, int sId, int *gids_arr, int size_of_gids_arr);
Info* Info_Insert(Info* T, int tm, int id, int* gids_arr, int size_of_gids_arr);
void printGroupInfo(Info *T);
int Universal_Hash_Function(int x);
SubInfo* Hash_LookUp(int id);
void Hash_Delete(int id);
TreeInfo* Consumption_Insert(TreeInfo* T, int id, int tm, SubInfo* sub, int k);
void pruneTree(Info *T, int tm, int k);
Info* Info_LookUp(Info* T, int id);
Info* Info_Delete(Info* T, int id);
int random(int min, int max);
void Consumption_Print(TreeInfo* T);
void freeInfo(Info *T);
void freeSub(Sub *T);
void freeConsumption(TreeInfo *T);
TreeInfo* Consumption_LookUp(TreeInfo* T, int tm, int id);

/**
 * @brief Optional function to initialize data structures that
 *        need initialization
 *
 * @param m Size of the hash table.
 * @param p Prime number for the universal hash functions.
 *
 * @return 0 on success
 *         1 on failure
 */
int initialize(int m, int p){
    int i;
    // Initializes Hash parameters
    P = p;
    M = m;
    A = random(1, P-1);
    B = random(0, P-1);
    // Initializes G
    for (i=0; i<MG; i++) {
        G[i].gId=i;
        G[i].gr=NULL;
        G[i].gsub=NULL;
    }
    return EXIT_SUCCESS;
}

/**
 * @brief Free resources
 *
 * @return 0 on success
 *         1 on failure
 */
int free_all(void){
    int i, j;
    SubInfo *p, *next;
    for (i=0; i<MG; i++) {
        // Free group's info tree
        freeInfo(G[i].gr);
        G[i].gr=NULL;
        // Free group's sub list
        freeSub(G[i].gsub);
    }
    // Free subinfo tree
    for (i = 0; i<M; i++) {
        p = HT[i]; // For each HT chain
        while (p!=NULL) { // Free sub info
            next=p->snext;
            for (j=0; j<MG; j++) { // Free Consumption Tree
                if (p->tgp[j]!=NULL && p->tgp[j]!= (TreeInfo*) 1) {
                    freeConsumption(p->tgp[i]);
                }
            }
            free(p); // Free Sub Info
            p=next;
        }
    }
    return EXIT_SUCCESS;
}

/**
 * @brief Insert info
 *
 * @param iTM Timestamp of arrival
 * @param iId Identifier of information
 * @param gids_arr Pointer to array containing the gids of the Event.
 * @param size_of_gids_arr Size of gids_arr including -1
 * @return 0 on success
 *          1 on failure
 */
int Insert_Info(int iTM,int iId,int* gids_arr,int size_of_gids_arr){
    int i;
    // Checks & fixes
    if (iTM<0 || iId<0 || size_of_gids_arr<=0 || !Info_isUnique_iId(iId, iTM)) return EXIT_FAILURE;
    filterArray(gids_arr, &size_of_gids_arr);
    // Insert info in groups of gids_arr
    for (i=0; i<size_of_gids_arr; i++) {
        if (gids_arr[i]!=-2)
            G[gids_arr[i]].gr= Info_Insert(G[gids_arr[i]].gr, iTM, iId, gids_arr, size_of_gids_arr);
    }
    // Print
    Insert_Info_Print(iTM, iId, gids_arr, size_of_gids_arr);
    return EXIT_SUCCESS;
}
/**
 * @brief Subsriber Registration
 *
 * @param sTM Timestamp of arrival
 * @param sId Identifier of subscriber
 * @param gids_arr Pointer to array containing the gids of the Event.
 * @param size_of_gids_arr Size of gids_arr including -1
 * @return 0 on success
 *          1 on failure
 */
int Subscriber_Registration(int sTM,int sId,int* gids_arr,int size_of_gids_arr) {
    int i;
    // Checks & fixes
    if (sTM<0 || sId <0 || size_of_gids_arr<=0 || !Subscriber_isUnique_sId(sId)) return EXIT_FAILURE;
    filterArray(gids_arr, &size_of_gids_arr);
    // Insert subscriber in groups of gids_arr
    for (i=0; i<size_of_gids_arr; i++) {
        if (gids_arr[i]!=-2) G[gids_arr[i]].gsub=Subscriber_Insert(G[gids_arr[i]].gsub, sId);
    }
    // Insert subscriber in Hash Table
    Hash_Insert(sTM, sId, gids_arr, size_of_gids_arr);
    // Print
    Subscriber_Registration_Print(sTM, sId, gids_arr, size_of_gids_arr);
    return EXIT_SUCCESS;
}
/**
 * @brief Prune Information from server and forward it to client
 *
 * @param tm Information timestamp of arrival
 * @return 0 on success
 *          1 on failure
 */
int Prune(int tm){
    int i, j;
    SubInfo *p;
    Info* info;
    Sub* sub;
    // Checks
    if (tm<0) return EXIT_FAILURE;
    // Printing is done simultaneously
    printf("R DONE\n");
    for (i = 0; i < MG; i++) {
        // Prune for Group
        printf("    GROUPID = %d, ", G[i].gId);
        pruneTree(G[i].gr, tm, i);
        // Print new group info list
        printf("INFOLIST:");
        info = G[i].gr;
        printGroupInfo(info);
        // Print group sub list
        printf(", SUBLIST: ");
        sub = G[i].gsub;
        while (sub!=NULL) {
            printf("%d ", sub->sId);
            sub=sub->snext;
        }
        printf("\n");
    }
    printf("\n");
    // Print sub info for each sub
    for (i=0; i<M; i++) {
        p=HT[i];
        while (p!=NULL) { // For subs in chain
            printf("    SUBSCRIBERID = %d, GROUPLIST =\n", p->sId);
            for (j=0; j<MG; j++) { // Print sub's new Consumption tree of every interested group
                if (p->tgp[j]!=(TreeInfo*) 1) {
                    printf("        %d, TREELIST =", G[j].gId);
                    Consumption_Print(p->tgp[j]);
                    printf("\n");
                }
            }
            printf("\n");
            p=p->snext;
        }
    }
    return EXIT_SUCCESS;
}

/**
 * @brief Consume Information for subscriber
 *
 * @param sId Subscriber identifier
 * @return 0 on success
 *          1 on failure
 */
int Consume(int sId){
    int i;
    TreeInfo* preConsume[MG];
    SubInfo *sub = getSub(sId);
    // Checks & fixes
    if (!isSubValid(sId)) return EXIT_FAILURE;
    // Keeps a copy of sgp array for the printing process later
    for (i=0; i<MG; i++) preConsume[i]=sub->sgp[i];
    // Consumes
    for (i=0; i<MG; i++) {
        if (sub->tgp[i]!=(TreeInfo*) 1) {
            sub->sgp[i]=ConsumeInfo(sub->tgp[i], (sub->sgp[i]==NULL)?NULL:sub->sgp[i]->prev);
        }
    }
    // Print
    Consume_Print(sub, preConsume);
    return EXIT_SUCCESS;
}

/**
 * @brief Delete subscriber
 *
 * @param sId Subscriber identifier
 * @return 0 on success
 *          1 on failure
 */
int Delete_Subscriber(int sId){
    int i;
    Info* gids_arr[MG];
    // Checks & fixes
    SubInfo* sub = Hash_LookUp(sId);
    if (sub==NULL) return EXIT_FAILURE;
    // Deletes sub from groups
    for (i=0; i<MG; i++) {
        // Keeps a copy of sub's interests for the printing process
        if (sub->tgp[i]!= (TreeInfo*) 1)
            gids_arr[i]=NULL;
        else
            gids_arr[i]= (Info*) 1;
        // Deletes them from their interested groups
        if (gids_arr[i]!= (Info*) 1)
            G[i].gsub = Subscriber_Delete(G[i].gsub, sId);
    }
    // Deletes sub from Hash Table
    Hash_Delete(sId);
    // Print
    Delete_Subscriber_Print(sId, gids_arr);
    return EXIT_SUCCESS;
}
/**
 * @brief Print Data Structures of the system
 *
 * @return 0 on success
 *          1 on failure
 */
int Print_all(void){
    int i, j, subs=0;
    Info* info;
    Sub* sub;
    SubInfo* subinfo;
    printf("P DONE\n");
    for (i=0; i<MG; i++) {
        // Prints group
        printf("    GROUPID = %d, INFOLIST=", G[i].gId);
        info=G[i].gr;
        printGroupInfo(info);
        printf(", SUBLIST =");
        sub=G[i].gsub;
        while(sub!=NULL) {
            printf(" %d", sub->sId);
            sub=sub->snext;
        }
        printf("\n");
    }
    // Prints sublist
    printf("    SUBSCRIBERLIST =");
    for (i = 0; i<M; i++) {
        subinfo = HT[i];
        while (subinfo != NULL) {
            printf(" %d", subinfo->sId);
            subinfo = subinfo->snext;
            subs++;
        }
    }
    printf("\n");
    // Prints SubInfo list
    for (i = 0; i<M; i++) {
        subinfo = HT[i];
        while (subinfo != NULL) {
            printf("    SUBSCRIBERID = %d, GROUPLIST =\n", subinfo->sId);
            for (j = 0; j < MG; j++) {
                if (subinfo->tgp[j] != (TreeInfo *) 1) {
                    printf("        %d, TREEINFO =", G[j].gId);
                    Consumption_Print(subinfo->tgp[j]);
                    printf("\n");
                }
            }
            subinfo = subinfo->snext;
        }
    }
    // Prints last line
    printf("    NO_GROUPS = %d, NO_SUBSCRIBERS = %d\n", MG, subs);
    return EXIT_SUCCESS;
}

// FUNCTIONS

/**
 * Consumes info from the LO Tree
 * @param info LO Tree
 * @param end Last consumption point (including itself)
 * @return Newest info
 */
TreeInfo* ConsumeInfo(TreeInfo *info, TreeInfo *end) {
    TreeInfo* p = info, *newest;
    if (info==NULL) return NULL;
    // Goes to newest
    while (p->trc!=NULL)
        p=p->trc;
    // Saves it to return it
    newest=p;
    // "Consumes"
    while (p!=end)
        p=p->prev;
    return newest;
}
/**
 * Returns the requested sub info
 * @param id Sub id
 * @return Sub info pointer or NUll if not found
 */
SubInfo *getSub(int id) {
    SubInfo *p;
    // Gets their chain (if they exist)
    p = Hash_LookUp(id);
    // Searches the chain
    while (p!=NULL) {
        if (p->sId==id) return p;
        p=p->snext;
    }
    return NULL;
}

/**
 * Deletes Info node from BST
 * @param T BS Tree
 * @param id Id to be removed
 * @return New Tree
 */
Info* Info_Delete(Info* T, int id) {
    Info* p, *tmp;
    // Checks if it exists
    p = Info_LookUp(T, id);
    if (p!=NULL) {
        if (p->ilc==NULL && p->irc==NULL) { // Is leaf
            if (T==p) { // Is root
                free(p);
                return NULL;
            }
            if (p->iId<p->ip->iId) // Is left child
                p->ip->ilc=NULL;
            else
                p->ip->irc=NULL; // Is right child
            free(p);
        } else if (p->ilc!=NULL && p->irc!=NULL) { // Has 2 children
            tmp = p->irc;               // Gets Inorder successor
            while (tmp->ilc!=NULL) {    // (One right and all left)
                tmp=tmp->ilc;
            }
            p->iId=tmp->iId; // Moves it to p
            if (tmp->irc != NULL) { // Gets successor's right children
                tmp->irc->ip = tmp->ip;
            }
            if (p->irc==tmp) { // If successor is p's right child
                tmp->ip->irc = tmp->irc;
            } else { // If successor is p's left child
                tmp->ip->ilc = tmp->irc;
            }
            free(tmp);
        } else { // Has 1 child
            if (p->ilc!=NULL) { // Has left child
                p->ilc->ip=p->ip;
                if (p->ip!=NULL && p->ip->iId>=p->iId) // Left child become father's left child
                    p->ip->ilc=p->ilc;
                else if (p->ip!=NULL && p->ip->iId<p->iId) // Right child become father's left child
                    p->ip->irc=p->ilc;
                else
                    T=p->ilc; // Child becomes root
            } else { // Has right child
                p->irc->ip=p->ip;
                if (p->ip!=NULL && p->ip->iId>=p->iId) // Right child become father's right child
                    p->ip->ilc=p->irc;
                else if (p->ip!=NULL && p->ip->iId<p->iId) // Left child become father's right child
                    p->ip->irc=p->irc;
                else
                    T=p->irc; // Child becomes root
            }
            free(p);
        }
    }
    return T;
}

/**
 * Inserts Info in sub's consumption tree
 * @param T LO Tree
 * @param id Info id
 * @param tm Info tm
 * @param sub Owner of the tree
 * @param k Group (This tree belongs to group k)
 * @return New LO Tree
 */
TreeInfo* Consumption_Insert(TreeInfo* T, int id, int tm, SubInfo* sub, int k) {
    TreeInfo *new, *p, *newrc, *newlc;
    if (T==NULL) { // Tree is empty
        new = (TreeInfo*) malloc(sizeof(TreeInfo));
        new->tlc=NULL; new->trc=NULL;
        new->tp=NULL;
        new->next=NULL; new->prev=NULL;
        new->tId=id;
        new->ttm=tm;
        return new;
    }
    p=T;
    while(p->tlc!=NULL) { // Find where to place it (like BST Search)
        if (p->ttm<tm)
            p=p->trc;
        else
            p=p->tlc;
    }
    // Create new children
    newlc = (TreeInfo*) malloc(sizeof(TreeInfo));
    newlc->tlc=NULL; newlc->trc=NULL;
    newrc = (TreeInfo*) malloc(sizeof(TreeInfo));
    newrc->tlc=NULL; newrc->trc=NULL;
    p->trc=newrc;
    p->tlc=newlc;
    // Has to be placed as right child
    if (p->ttm<tm) {
        // RC=new node
        newrc->tp = p;
        newrc->tId = id;
        newrc->ttm = tm;
        newrc->next = p->next;
        if (p->next != NULL) p->next->prev = newrc;
        newrc->prev = newlc;
        // LC=father
        newlc->tp = p;
        newlc->tId = p->tId;
        newlc->ttm = p->ttm;
        newlc->next = newrc;
        newlc->prev = p->prev;
        if (p->prev != NULL) p->prev->next = newlc;
        // Fix sgp's pointer (consumption pointer)
        if (sub->sgp[k]==p) {
            sub->sgp[k]=newlc;
        }
    } else {
        // LC = new node
        newlc->tp=p;
        newlc->tId=id;
        newlc->ttm=tm;
        newlc->next=newrc;
        newlc->prev=p->prev;
        if (p->prev!=NULL) p->prev->next=newlc;
        // RC = father
        newrc->tp=p;
        newrc->tId=p->tId;
        newrc->ttm=p->ttm;
        newrc->next=p->next;
        if (p->next!=NULL) p->next->prev=newrc;
        newrc->prev=newlc;
        // Father=LC
        p->tId=id;
        p->ttm=tm;
        // Fix sgp's pointer (consumption pointer)
        if (sub->sgp[k]==p) {
            sub->sgp[k]=newrc;
        }
    }
    // Remove father's list connections
    p->next=NULL;
    p->prev=NULL;
    return T;
}

/**
 * Search a BS Tree
 * @param T BS Tree
 * @param id Id to search
 * @return Pointer of id if found
 */
Info* Info_LookUp(Info* T, int id) {
    if (T==NULL)
        return NULL;
    if (T->iId==id)
        return T;
    else if (T->iId>id)
        return Info_LookUp(T->ilc, id);
    else
        return Info_LookUp(T->irc, id);
}

/**
 * Prunes the given tree node (Whole tree for recursion)
 * @param T BS Tree to be pruned
 * @param tm TM of pruning
 * @param k Group the tree belongs to
 */
void pruneTree(Info *T, int tm, int k) {
    Sub* sub = NULL;
    SubInfo *si = NULL;
    // Tree is empty
    if (T==NULL)
        return;
    else {
        // Prune post-orderly
        pruneTree(T->ilc, tm, k);
        pruneTree(T->irc, tm, k);
        // Prune condition
        if (T->itm <= tm) {
            // Get tree's sublist
            sub = G[k].gsub;
            // Add pruned info to every sub in the list
            while (sub != NULL) {
                si = getSub(sub->sId);
                si->tgp[k] = Consumption_Insert(si->tgp[k], T->iId, T->itm, si, k);
                sub = sub->snext;
            }
            // After adding the pruned node to sub's consumption tree,
            // delete it from the group's tree
            G[k].gr = Info_Delete(G[k].gr, T->iId);
        }
    }
}

/**
 * Check if id exists
 * @param sId Id to be searched
 * @return True if it exists
 */
bool isSubValid(int sId) {
    SubInfo* p= Hash_LookUp(sId);
    if (p==NULL)
        return false;
    else
        return true;
}

/**
 * Removes a subscriber from the chain
 * @param List Hash Table Chain
 * @param id Id to be removed
 * @return New chain
 */
SubInfo *SubInfo_Delete(SubInfo* List, int id) {
    SubInfo *tmp = List, *prev = NULL, *del = NULL;
    while(tmp!=NULL && tmp->sId!=id) {
        prev=tmp;
        tmp=tmp->snext;
    }
    if (tmp!=NULL) {
        if (tmp==List) {
            del = List;
            List = List->snext;
            free(del);
        } else {
            del = tmp;
            prev->snext=tmp->snext;
            free(del);
        }
    }
    return List;
}

/**
 * Searches for the given sub
 * @param List Chain to be searched
 * @param id Id to be searched
 * @return Sub Info pointer if it exists
 */
SubInfo* SubInfo_LookUp(SubInfo* List, int id) {
    SubInfo *tmp = List;
    while(tmp!=NULL && tmp->sId!=id) {
        tmp=tmp->snext;
    }
    if (tmp!=NULL) return tmp;
    else return NULL;
}

/**
 * Deletes a sub id from the Hash Table
 * @param id Id to be removed
 */
void Hash_Delete(int id) {
    int p;
    p = Universal_Hash_Function(id);
    HT[p] = SubInfo_Delete(HT[p], id);
}

/**
 * Searches for a sub in the Hash Table
 * @param id Id of the sub
 * @return SubInfo pointer of sub if it exists
 */
SubInfo* Hash_LookUp(int id) {
    SubInfo *p, *list;
    int index;
    index = Universal_Hash_Function(id);
    list = HT[index];
    p = SubInfo_LookUp(list, id);
    return p;
}

/**
 * Removes subscriber from a sub list
 * @param List Sub list
 * @param id Id of the subscriber
 * @return New sub list
 */
Sub *Subscriber_Delete(Sub *List, int id) {
    Sub *tmp = List, *prev = NULL, *del = NULL;
    // Finds node
    while(tmp!=NULL && tmp->sId!=id) {
        prev=tmp;
        tmp=tmp->snext;
    }
    // Deletes it (if it finds it)
    if (tmp!=NULL) {
        if (tmp==List) {
            del = List;
            List = List->snext;
            free(del);
        } else {
            del = tmp;
            prev->snext=tmp->snext;
            free(del);
        }
    }
    return List;
}

/**
 * Inserts a subscriber in the Hash Table
 * @param sTM Sub's tm
 * @param sId Sub's id
 * @param gids_arr Groups he's interested to
 * @param size_of_gids_arr Size of gids_arr
 */
void Hash_Insert(int sTM, int sId, int *gids_arr, int size_of_gids_arr) {
    int index;
    index = Universal_Hash_Function(sId);
    HT[index] = SubInfo_Insert(HT[index], sTM, sId, gids_arr, size_of_gids_arr);
}

/**
 * Inserts subscriber to a sub list
 * @param List Sub list
 * @param id Id of the subscriber
 * @return New sub list
 */
Sub* Subscriber_Insert(struct Subscription *List, int id) {
    Sub *new, *tmp=List, *prev=NULL;
    // Creates new node
    new = (Sub *) malloc(sizeof(Sub));
    new->sId=id;
    // Sorts (finds where to insert it)
    while (tmp!=NULL && tmp->sId<id) {
        prev=tmp;
        tmp=tmp->snext;
    }
    // Inserts it
    if (tmp==List) {
        new->snext=List;
        return new;
    } else {
        new->snext=prev->snext;
        prev->snext=new;
        return List;
    }
}

/**
 * Checks if sub id is unique
 * @param id Id to be checked
 * @return True if it is unique
 */
bool Subscriber_isUnique_sId(int id) {
    SubInfo* p = getSub(id);
    if (p==NULL) return true;
    return false;
}

/**
 * Inserts a subscriber to the Sub Info chain
 * @param List Chain to be inserted into
 * @param tm Sub's tm
 * @param id Sub's id
 * @param gids_arr Groups he's interested to
 * @param size_of_gids_arr Size of gids_arr
 * @return New SubInfo chain
 */
SubInfo *SubInfo_Insert(SubInfo *List, int tm, int id, int *gids_arr, int size_of_gids_arr) {
    SubInfo *new, *tmp=List, *prev=NULL;
    int i;
    // Creates new node
    new = (SubInfo *) malloc(sizeof(SubInfo));
    new->sId=id;
    new->stm=tm;
    for (i=0; i<MG; i++) {
        if (SubInfo_Interested(gids_arr, size_of_gids_arr, i)) {
            new->sgp[i]=NULL;
            new->tgp[i]=NULL;
        }
        else {
            new->sgp[i]= (struct TreeInfo *) 1;
            new->tgp[i]= (struct TreeInfo *) 1;
        }
    }
    // Sorts (finds where to insert it)
    while (tmp!=NULL && tmp->sId<id) {
        prev=tmp;
        tmp=tmp->snext;
    }
    // Inserts it
    if (tmp==List) {
        new->snext=List;
        return new;
    } else {
        new->snext=prev->snext;
        prev->snext=new;
        return List;
    }
}

/**
 * Checks if k is contained in gids_arr
 * @param gids_arr Groups array
 * @param size_of_gids_arr size of gids_arr
 * @param k Group to be checked if it's contained
 * @return True if it contained
 */
bool SubInfo_Interested(const int* gids_arr, int size_of_gids_arr, int k) {
    int i;
    for (i=0; i<size_of_gids_arr; i++) {
        if (gids_arr[i]==k) return true;
    }
    return false;
}

/**
 * Insert Info in BS Tree
 * @param T BS Tree
 * @param tm Info tm
 * @param id Info id
 * @param gids_arr Groups the info is associated with
 * @param size_of_gids_arr Size of gids_arr
 * @return New Info BS Tree
 */
Info* Info_Insert(Info* T, int tm, int id, int* gids_arr, int size_of_gids_arr) {
    Info* p = T, *par = NULL, *new;
    int i;
    // Find where to insert it (Like BST Search)
    while(p!=NULL) {
        par=p;
        if (p->iId>id) {
            p=p->ilc;
        } else {
            p=p->irc;
        }
    }
    // Create new node
    new = (Info *) malloc(sizeof(Info));
    new->iId=id;
    new->itm=tm;
    for(i=0; i<MG; i++) {
        if (SubInfo_Interested(gids_arr, size_of_gids_arr, i)) new->igp[i]= 1;
        else new->igp[i]=0;
    }
    // Insert it in tree
    new->ilc=NULL;
    new->irc=NULL;
    new->ip=par;
    if (par!=NULL && par->iId>=id) // Placed as left child
        par->ilc=new;
    else if (par!=NULL && par->iId<id) // Placed as right child
        par->irc=new;
    else {
        return new; // Is root
    }
    return T;
}

/**
 * Checks if info id is unique
 * @param id Id to be checked
 * @return True if it is unique
 */
bool Info_isUnique_iId(int id, int tm) {
    int i, j;
    Info *p;
    SubInfo *si;
    TreeInfo *ti;
    // Checks the info list of every group
    for (i = 0; i < MG; i++) {
        p = G[i].gr;
        p = Info_LookUp(p, id);
        if (p!=NULL) return false;
    }
    // Checks the consumption tree of every sub
    for (i = 0; i<M; i++) {
        si = HT[i];
        while (si!=NULL) {
            for (j = 0; j<MG; j++) {
                ti = si->tgp[i];
                if (ti!=(TreeInfo*) 1 && Consumption_LookUp(ti, tm, id)!=NULL) {
                    return false;
                }
            }
            si = si->snext;
        }
    }
    return true;
}


TreeInfo* Consumption_LookUp(TreeInfo* T, int tm, int id) {
    TreeInfo *p=T;
    while(p!=NULL && p->ttm!=tm) {
        if (p->tlc==NULL && p->tId==id)
            return p;
        else if (p->ttm<tm)
            p=p->trc;
        else
            p=p->tlc;
    }
    return NULL;
}

// PRINT

/**
 * Handles printing process after consume event
 * @param sub Sub who requested consume
 * @param preConsume Their Consumption tree before consuming
 */
void Consume_Print(SubInfo *sub, TreeInfo** preConsume) {
    int i, id;
    id = sub->sId;
    printf("C %d DONE\n", id);
    for (i=0; i<MG; i++) {
        if (preConsume[i] != (TreeInfo *) 1) {
            printf("    GROUPID = %d, TREELIST =", G[i].gId);
            Consume_Print_Info(sub->tgp[i], (preConsume[i]==NULL)?NULL:preConsume[i]->prev);
            printf(", NEWGP = ");
            if (sub->sgp[i]!=NULL)
                printf("%d", sub->sgp[i]->tId);
            printf("\n");
        }
    }
}

/**
 * Print consumption tree until a certain point (newer to older)
 * @param info Consumption Tree
 * @param end Ending point (including it)
 */
void Consume_Print_Info(TreeInfo *info, TreeInfo *end) {
    TreeInfo* p = info;
    if (p==NULL) return;
    while (p->trc != NULL)
        p = p->trc;
    while (p != end) {
        printf(" %d", p->tId);
        p = p->prev;
    }
}

/**
 * Print info list of a consumption tree
 * @param T Consumption Tree
 */
void Consumption_Print(TreeInfo* T) {
    TreeInfo *p = T;
    if (T==NULL) return;
    while (p->tlc!=NULL) p=p->tlc;
    while (p!=NULL) {
        printf(" %d", p->tId);
        p=p->next;
    }
}

/**
 * Handles printing process after a subscriber deletion event
 * @param sId Sub id deleted
 * @param gids_arr Sub's igp
 */
void Delete_Subscriber_Print(int sId, Info **gids_arr) {
    int i;
    Sub* s;
    SubInfo* ptr;
    printf("D %d DONE\n", sId);
    printf("    SUBSCRIBERLIST =");
    for (i=0; i<M; i++) {
        ptr = HT[i];
        while (ptr!=NULL) {
            printf(" %d", ptr->sId);
            ptr=ptr->snext;
        }
    }
    printf("\n");
    for (i=0; i<MG; i++) {
        if (gids_arr[i] != (Info*) 1) {
            printf("    GROUPID = %d, SUBLIST =", G[i].gId);
            s = G[i].gsub;
            while (s != NULL) {
                printf(" %d", s->sId);
                s = s->snext;
            }
            printf("\n");
        }
    }
}

/**
 * Handles printing process after a subscriber registration event
 * @param sTM Sub's tm
 * @param sId Sub's id
 * @param gids_arr Sub's groups of interested
 * @param size_of_gids_arr Size of gids_arr
 */
void Subscriber_Registration_Print(int sTM, int sId, int *gids_arr, int size_of_gids_arr) {
    int i;
    Sub* s;
    SubInfo* ptr;
    printf("S %d %d", sTM, sId);
    for (i=0; i<MG; i++) {
        if (SubInfo_Interested(gids_arr, size_of_gids_arr, i))
            printf(" %d", G[i].gId);
    }
    printf(" DONE\n");
    printf("    SUBSCRIBERLIST = ");
    for (i=0; i<M; i++) {
        ptr = HT[i];
        while (ptr!=NULL) {
            printf(" %d", ptr->sId);
            ptr=ptr->snext;
        }
    }
    printf("\n");
    for (i=0; i<MG; i++) {
        if (gids_arr[i]!=-2 && SubInfo_Interested(gids_arr, size_of_gids_arr, i)) {
            printf("    GROUPID = %d, SUBLIST =", G[i].gId);
            s = G[i].gsub;
            while (s != NULL) {
                printf(" %d", s->sId);
                s = s->snext;
            }
            printf("\n");
        }
    }
}

/**
 * Prints group's info tree
 * @param T Info list
 */
void printGroupInfo(Info *T) {
    if (T==NULL) return;
    printGroupInfo(T->ilc);
    printf(" %d", T->iId);
    printGroupInfo(T->irc);
}

/**
 * Handles printing process after an info insertion event
 * @param iTM Info tm
 * @param iId Info id
 * @param gids_arr Groups the info is associated with
 * @param size_of_gids_arr Size of gids_arr
 */
void Insert_Info_Print(int iTM,int iId, const int *gids_arr, int size_of_gids_arr) {
    int i;
    Info* p;
    printf("I %d %d DONE\n", iTM, iId);
    for (i=0; i<size_of_gids_arr; i++) {
        if (gids_arr[i]!=-2) {
            printf("    GROUPID = %d, INFOLIST =", G[gids_arr[i]].gId);
            p = G[gids_arr[i]].gr;
            printGroupInfo(p);
            printf("\n");
        }
    }
}
// UTILITY

/**
 * Filters gids_arr array: Invalid values are set to -2
 * @param gids_arr gids_arr to be filtered
 * @param size_of_gids_arr Size of gids_arr
 */
void filterArray(int *gids_arr, int *size_of_gids_arr) {
    int i, j;
    *size_of_gids_arr=*size_of_gids_arr-1;
    int n=*size_of_gids_arr;
    // Checks for duplicates
    for (i=0; i<n; i++) {
        if (gids_arr[i]>=M || gids_arr<0)
            gids_arr[i]=-2;
        for (j=i+1; j<n; j++) {
            if (gids_arr[i]==gids_arr[j]) {
                gids_arr[j]=-2; // Duplicate number is set to -2: A non-valid Id that marks this cell as "empty"/non-valid
            }
        }
    }
}

/**
 * Returns a random number between min and max (including them)
 * @param min Low border
 * @param max High border
 * @return Random number
 */
int random(int min, int max) {
    srand(time(0));
    int num = (rand() % (min - max + 1)) + min;
    return num;
}

/**
 * Returns the index of Hash Table for number x based on hash function
 * @param x X's index requested
 * @return Hash Table's index for X
 */
int Universal_Hash_Function(int x) {
    int k = ((A*x+B) % P) % M;
    return k;
}

/**
 * Frees consumption tree
 * @param T Consumption tree
 */
void freeConsumption(TreeInfo *T) {
    if (T==NULL || T==(TreeInfo*) 1) return;
    freeConsumption(T->tlc);
    freeConsumption(T->trc);
    if (T==NULL || T==(TreeInfo*) 1) free(T); // Used to avoid some unexpected "duplicated frees"
}

/**
 * Free group's sub list
 * @param T Group's sub list
 */
void freeSub(Sub *T) {
    Sub* p = T, *del;
    while (p!=NULL) {
        del = p;
        p=p->snext;
        free(del);
    }
}

/**
 * Free group's info list
 * @param T Group's info list
 */
void freeInfo(Info *T) {
    if (T==NULL) return;
    freeInfo(T->ilc);
    freeInfo(T->irc);
    free(T);
}