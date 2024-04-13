/***************************************************************
 *
 * file: pss.h
 *
 * @Author  Nikolaos Vasilikopoulos (nvasilik@csd.uoc.gr)
 * @Version 20-10-2020
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
#include <stdbool.h>
#include "pss.h"

Group* G = NULL;
SubInfo* SI = NULL;

Info* Info_Insert(Info *List, int tm, int id, int* gids_arr, int size_of_gids_arr, Info **LastR);
bool Info_isUnique_iId(int id);
bool Subscriber_isUnique_sId(int id);
Sub* Subscriber_Insert(Sub* List, int tm, int id);
Sub* Subscriber_Delete(Sub* List, int id);
SubInfo *SubInfo_Insert(SubInfo *List, int tm, int id, int *gids_arr, int size_of_gids_arr);
bool SubInfo_Interested(const int* gids_arr, int size_of_gids_arr, int k);
SubInfo *SubInfo_Delete(SubInfo *List, int id);
void ConsumeInfo(Info *info, Info* end);
void Insert_Info_Print(int iTM,int iId, const int *gids_arr, int size_of_gids_arr);
void Delete_Subscriber_Print(int sId, Info **gids_arr);
void Subscriber_Registration_Print(int sTM, int sId, int *gids_arr, int size_of_gids_arr);
void Consume_Print(int sId, Info** preConsume);
void Consume_Print_Info(Info *info, Info *end);
SubInfo *getSub(int id);
void removeDuplicates(int *gids_arr, int *size_of_gids_arr);
bool isGidsArrValid(const int *gids_arr, int size_of_gids_arr);
bool isSubValid(int sId);

/**
 * @brief Optional function to initialize data structures that
 *        need initialization
 *
 * @return 0 on success
 *         1 on failure
 */
int initialize(void){
    int i;
    // Initializes G
    G = (Group*) malloc(MG*sizeof(Group));
    for (i=0; i<MG; i++) {
        G[i].gId=i;
        G[i].gfirst=NULL; G[i].glast=NULL;
        G[i].ggsub=NULL;
    }
    return EXIT_SUCCESS;
}

/**
 * @brief Free resources
 *
 * @return 0 on success
 *         1 on failure
 */
int free_all(void) {
    int i;
    Sub *sub = G[0].ggsub; SubInfo *si = SI; Info *info = G[0].gfirst;
    Sub *sub_del; SubInfo *si_del; Info *info_del;
    for (i=0; i<MG; i++) {
        // Free group's info list
        if (info!=NULL) {
            info_del = info;
            info = info->inext;
            free(info_del);
        }
        // Free group's sub list
        if (sub!=NULL) {
            sub_del = sub;
            sub = sub->snext;
            free(sub_del);
        }
    }
    // Free subinfo list
    while(si!=NULL) {
        si_del = si;
        si = si->snext;
        free(si_del);
    }
    free(G);
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
int Insert_Info(int iTM,int iId,int* gids_arr,int size_of_gids_arr) {
    int i;
    // Checks & fixes
    if (iTM<0 || iId<0 || size_of_gids_arr<=0 || !isGidsArrValid(gids_arr, size_of_gids_arr)) return EXIT_FAILURE;
    removeDuplicates(gids_arr, &size_of_gids_arr);
    if (!Info_isUnique_iId(iId)) return EXIT_FAILURE;
    // Insert info in groups of gids_arr
    for (i=0; i<size_of_gids_arr; i++) {
        if (gids_arr[i]!=-2) G[gids_arr[i]].gfirst=Info_Insert(G[gids_arr[i]].gfirst, iTM, iId, gids_arr, size_of_gids_arr, &G[gids_arr[i]].glast);
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
    if (sTM<0 || sId <0 || size_of_gids_arr<=0 || !isGidsArrValid(gids_arr, size_of_gids_arr)) return EXIT_FAILURE;
    removeDuplicates(gids_arr, &size_of_gids_arr);
    if (!Subscriber_isUnique_sId(sId)) return EXIT_FAILURE;
    // Insert subscriber in groups of gids_arr
    for (i=0; i<size_of_gids_arr; i++) {
            if (gids_arr[i]!=-2) G[gids_arr[i]].ggsub=Subscriber_Insert(G[gids_arr[i]].ggsub, sTM, sId);
    }
    // Insert subscriber in SubInfo list
    SI=SubInfo_Insert(SI, sTM, sId, gids_arr, size_of_gids_arr);
    // Print
    Subscriber_Registration_Print(sTM, sId, gids_arr, size_of_gids_arr);
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
    Info* preConsume[MG];
    SubInfo *sub = getSub(sId);
    // Checks & fixes
    if (!isSubValid(sId)) return EXIT_FAILURE;
    // Keeps a copy of sgp array for the printing proccess later
    for (i=0; i<MG; i++) preConsume[i]=sub->sgp[i];
    // Consumes
    for (i=0; i<MG; i++) {
        if (sub->sgp[i]!=(Info*) 1) {
            ConsumeInfo(G[i].gfirst, (sub->sgp[i]==NULL)?NULL:sub->sgp[i]->inext);
            sub->sgp[i]=G[i].gfirst;
        }
    }
    // Print
    Consume_Print(sId, preConsume);
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
    if (!isSubValid(sId)) return EXIT_FAILURE;
    SubInfo *sub = getSub(sId);
    // Deletes sub from groups
    for (i=0; i<MG; i++) {
        gids_arr[i]=sub->sgp[i];
        G[i].ggsub = Subscriber_Delete(G[i].ggsub, sId);
    }
    // Deletes sub from SubInfo list
    SI = SubInfo_Delete(SI, sId);
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
    int i, subs=0;
    Info* info;
    Sub* sub;
    SubInfo* subinfo;
    printf("P DONE\n");
    for (i=0; i<MG; i++) {
        // Prints group
        printf("    GROUPID = %d, INFOLIST=", G[i].gId);
        info=G[i].gfirst;
        while(info!=NULL) {
            printf(" %d", info->iId);
            info=info->inext;
        }
        printf(", SUBLIST =");
        sub=G[i].ggsub;
        while(sub!=NULL) {
            printf(" %d", sub->sId);
            sub=sub->snext;
        }
        printf("\n");
    }
    // Prints sublist
    printf("    SUBSCRIBERLIST =");
    subinfo = SI;
    while(subinfo!=NULL) {
        printf(" %d", subinfo->sId);
        subinfo=subinfo->snext;
        subs++;
    }
    printf("\n");
    // Prints SubInfo list
    subinfo = SI;
    while(subinfo!=NULL) {
        printf("    SUBSCRIBERID = %d, GROUPLIST=", subinfo->sId);
        for (i=0; i<MG; i++) {
            if (subinfo->sgp[i]!=(Info*) 1) printf(" %d", G[i].gId);
        }
        subinfo=subinfo->snext;
        printf("\n");
    }
    // Prints last line
    printf("    NO_GROUPS = %d, NO_SUBSCRIBERS = %d\n", MG, subs);
    return EXIT_SUCCESS;
}

// PRINT FUNCTIONS
/**
 * Print event after new info arrival
 * @param iTM Timestamp of the info arrived
 * @param iId Id of the info arrived
 * @param gids_arr Array of the groups affected by the info
 * @param size_of_gids_arr Size of gids_arr
 */
void Insert_Info_Print(int iTM,int iId, const int *gids_arr, int size_of_gids_arr) {
    int i;
    Info* p;
    printf("I %d %d DONE\n", iTM, iId);
    for (i=0; i<size_of_gids_arr; i++) {
        if (gids_arr[i]!=-2) {
            printf("    GROUPID = %d, INFOLIST =", G[gids_arr[i]].gId);
            p = G[gids_arr[i]].gfirst;
            while (p != NULL) {
                printf(" %d", p->iId);
                p = p->inext;
            }
            printf("\n");
        }
    }
}

/**
 * Print even after a subscriber removal
 * @param sId The Id of the deleted subscriber
 * @param gids_arr Contains the group were the subscriber was subscribed
 */
void Delete_Subscriber_Print(int sId, Info **gids_arr) {
    int i;
    Sub* s;
    SubInfo* si = SI;
    printf("D %d DONE\n", sId);
    printf("    SUBSCRIBERLIST =");
    while (si!=NULL) {
        printf(" %d", si->sId);
        si=si->snext;
    }
    printf("\n");
    for (i=0; i<MG; i++) {
        if (gids_arr[i] != (Info*) 1) {
            printf("    GROUPID = %d, SUBLIST =", G[i].gId);
            s = G[i].ggsub;
            while (s != NULL) {
                printf(" %d", s->sId);
                s = s->snext;
            }
            printf("\n");
        }
    }
}

/**
 * Print even after a subscriber registration
 * @param sTM The timestamp of the subscriber
 * @param sId The Id of the subscriber
 * @param gids_arr Contains the pointers of the groups the subscriber is interested for
 * @param size_of_gids_arr The size of gids_arr
 */
void Subscriber_Registration_Print(int sTM, int sId, int *gids_arr, int size_of_gids_arr) {
    int i;
    Sub* s;
    SubInfo* si = SI;
    printf("S %d %d", sTM, sId);
    for (i=0; i<MG; i++) {
        if (SubInfo_Interested(gids_arr, size_of_gids_arr, i)) printf(" %d", G[i].gId);
    }
    printf(" DONE\n");
    printf("    SUBSCRIBERLIST =");
    while (si!=NULL) {
        printf(" %d", si->sId);
        si=si->snext;
    }
    printf("\n");
    for (i=0; i<MG; i++) {
        if (gids_arr[i]!=-2 && SubInfo_Interested(gids_arr, size_of_gids_arr, i)) {
            printf("    GROUPID = %d, SUBLIST =", G[i].gId);
            s = G[i].ggsub;
            while (s != NULL) {
                printf(" %d", s->sId);
                s = s->snext;
            }
            printf("\n");
        }
    }
}

/**
 * Print event after a consume event
 * @param sId The Id of the consumer
 * @param preConsume Contains the pointer of each group for the subscriber before consuming
 */
void Consume_Print(int sId, Info** preConsume) {
    int i;
    printf("C %d DONE\n", sId);
    for (i=0; i<MG; i++) {
        if (preConsume[i] != (struct Info *) 1) {
            printf("    GROUPID = %d, INFOLIST =", G[i].gId);
            Consume_Print_Info(G[i].gfirst, (preConsume[i]==NULL)?NULL:preConsume[i]->inext);
            printf(", NEWGP = %p", G[i].gfirst);
            printf("\n");
        }
    }
}

/**
 * Recursively printing consumed info Ids (older to newer)
 * @param info Current info position
 */
void Consume_Print_Info(Info *info, Info *end) {
    if (info==end) return;
    Consume_Print_Info(info->inext, end);
    printf(" %d", info->iId);
}

// HELPER FUNCTIONS
/**
 * Checks if the info Id given is unique
 * @param id Id to be checked
 * @return true if it is unique, false if it is not
 */
bool Info_isUnique_iId(int id) {
    int i;
    Info* p;
    // Checks the info list of every group
    for (i=0; i<MG; i++) {
            p = G[i].gfirst;
            while (p != NULL) {
                if (p->iId == id) return false;
                p = p->inext;
            }
    }
    return true;
}

/**
 * Inserts the new info in the info list given
 * @param List The info list
 * @param tm Timestamp of the info
 * @param id Id of the info
 * @param gids_arr The list of groups to add that info in
 * @param size_of_gids_arr Size of gids_arr
 * @param LastR Reference to the last item of the list, so it can be updated if the new one is going to be added at the end of the list
 * @return New head of the list
 */
Info* Info_Insert(struct Info *List, int tm, int id, int* gids_arr, int size_of_gids_arr, struct Info **LastR) {
    int i;
    Info *new, *tmp=List;
    // Creates new node
    new = (Info *) malloc(sizeof(Info));
    new->iId=id;
    new->itm=tm;
    new->igp = (int *) malloc(MG*sizeof(int));
    for(i=0; i<MG; i++) {
        if (SubInfo_Interested(gids_arr, size_of_gids_arr, i)) new->igp[i]= 1;
        else new->igp[i]=0;
    }
    // Sorts (finds where to insert it)
    while (tmp!=NULL && tmp->inext!=NULL && tmp->inext->itm>tm) tmp=tmp->inext;
    // Inserts it
    if (tmp==List && ((List!=NULL && tmp->itm<tm) || List==NULL)) {
        new->inext=List;
        new->iprev=NULL;
        if (new->inext!=NULL) new->inext->iprev=new;
        if (new->inext==NULL) *LastR=new;
        return new;
    } else if (tmp->inext==NULL && tmp->itm>tm) {
        new->inext=NULL;
        new->iprev=tmp;
        tmp->inext=new;
    } else {
        new->inext=tmp->inext;
        new->iprev=tmp;
        tmp->inext=new;
        new->inext->iprev=new;
    }
    if (new->inext==NULL) *LastR=new;
    return List;
}

/**
 * Inserts the new subscriber in the subscribers list given
 * @param List The subscribers list
 * @param tm Timestamp of the subscriber
 * @param id Id of the subscriber
 * @return New head of the list
 */
Sub* Subscriber_Insert(struct Subscription *List, int tm, int id) {
    Sub *new, *tmp=List, *prev=NULL;
    // Creates new node
    new = (Sub *) malloc(sizeof(Sub));
    new->sId=id;
    new->stm=tm;
    // Sorts (finds where to insert it)
    while (tmp!=NULL && tmp->stm>tm) {
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
 * Checks if the given subscriber Id is unique
 * @param id Id to be checked
 * @return True if it unique, false if it is not
 */
bool Subscriber_isUnique_sId(int id) {
    int i;
    Sub* p;
    // Checks subscriber list of every group
    for (i=0; i<MG; i++) {
         p = G[i].ggsub;
         while (p != NULL) {
            if (p->sId == id) return false;
            p = p->snext;
        }
    }
    return true;
}

/**
 * Deletes subscriber from the given list
 * @param List List from which the subscriber is going to be deleted
 * @param id Id of the subscriber
 * @return New head of the list
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
 * Inserts new subscriber in the given subscriber info list
 * @param List List on which the subscriber is going to be added
 * @param tm Timestamp of the subscriber
 * @param id Id of the subscriber
 * @param gids_arr Contains info about the groups the subscriber is interested in
 * @param size_of_gids_arr The size of gids_arr
 * @return New head of the list
 */
SubInfo *SubInfo_Insert(SubInfo *List, int tm, int id, int *gids_arr, int size_of_gids_arr) {
    SubInfo *new, *tmp=List, *prev=NULL;
    int i;
    // Creates new node
    new = (SubInfo *) malloc(sizeof(SubInfo));
    new->sId=id;
    new->stm=tm;
    new->sgp = (Info **) malloc(MG*sizeof(Info*));
    for (i=0; i<MG; i++) {
        if (SubInfo_Interested(gids_arr, size_of_gids_arr, i)) new->sgp[i]=G[i].gfirst;
        else new->sgp[i]= (struct Info *) 1;
    }
    // Sorts (finds where to insert it)
    while (tmp!=NULL && tmp->stm<tm) {
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
 * Deletes a subscriber from the given subscriber info list
 * @param List The list from which the subscriber is going to be deleted
 * @param id The Id of the subscriber
 * @return New head of the list
 */
SubInfo *SubInfo_Delete(SubInfo *List, int id) {
    SubInfo *tmp = List, *prev = NULL, *del = NULL;
    // Finds node
    while(tmp!=NULL && tmp->sId!=id) {
        prev=tmp;
        tmp=tmp->snext;
    }
    // Deletes node
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
 * Checks whether the subscriber is interested about a specific group
 * @param gids_arr Contains info about the groups the subscriber is interested in
 * @param size_of_gids_arr The size of gids_arr
 * @param k The group to be checked
 * @return True if they are interested, false if not
 */
bool SubInfo_Interested(const int* gids_arr, int size_of_gids_arr, int k) {
    int i;
    for (i=0; i<size_of_gids_arr; i++) {
        if (gids_arr[i]==k) return true;
    }
    return false;
}

/**
 * Consumes info (reads list) until a certain point, recursively
 * @param info Info list to be consumed
 * @param end Where to stop consuming
 */
void ConsumeInfo(Info *info, Info *end) {
    if (info!=end) {
        ConsumeInfo(info->inext, end);
    }
}

/**
 * Gets the SubInfo pointer of a subscriber
 * @param id The Id of the requested subscriber
 * @return The SubInfo pointer of the requested subscriber
 */
SubInfo *getSub(int id) {
    SubInfo *p=SI;
    while (p!=NULL) {
        if (p->sId==id) return p;
        p=p->snext;
    }
    return NULL;
}

/**
 * Removes duplicate numbers (group ids) from an array (the gids array).
 * Duplicate ids add info, subscribers etc more than 1 times, so they have to be removed
 * @param gids_arr The gids_arr to be filtered
 * @param size_of_gids_arr The size of gids_arr
 */
void removeDuplicates(int *gids_arr, int *size_of_gids_arr) {
    int i, j;
    *size_of_gids_arr=*size_of_gids_arr-1;
    int n=*size_of_gids_arr;
    // Checks for duplicates
    for (i=0; i<n; i++) {
        for (j=i+1; j<n; j++) {
            if (gids_arr[i]==gids_arr[j]) {
                gids_arr[j]=-2; // Duplicate number is set to -2: A non-valid Id that marks this cell as "empty"/non-valid
            }
        }
    }
}

/**
 * Checks an array (the gids array) for negative numbers (ids)
 * @param gids_arr The gids_arr to be checked
 * @param size_of_gids_arr The size of gids_arr
 * @return True if there are no negative ids, false if there is at least one
 */
bool isGidsArrValid(const int *gids_arr, int size_of_gids_arr) {
    int i;
    for (i=0; i<size_of_gids_arr-1; i++) {
        if (gids_arr[i]<0 || gids_arr[i]>=MG) return false;
    }
    return true;
}

/**
 * Checks whether the subscriber id is valid (exists in the system)
 * @param sId The subscriber Id to be checked
 * @return True if it is exists, false if it does not
 */
bool isSubValid(int sId) {
    SubInfo* p=SI;
    while(p!=NULL) {
        if (p->sId==sId) return true;
        p=p->snext;
    }
    return false;
}