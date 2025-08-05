/** @file
 * @brief GRIB table names.
 * @author Public Domain: Wesley Ebisuzaki @date 2006
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2006 | W. Ebisuzaki | Initial
 * 4/2007 | W. Ebisuzaki | Added netCDF support
 * 6/2011 | W. Ebisuzaki | Made parameter category >= 192 local
 * 2/2012 | W. Ebisuzaki | Fixed search_gribtab for local tables
 * 4/2013 | W. Ebisuzaki | gribtab -> gribtable, added user_gribtable
 * 1/2021 | W. Ebisuzaki | Use NCEP or ECMWF names as default, local table handled epar
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"

/** NCEP Grib Table */
extern struct gribtable_s NCEP_gribtable[];
/** ECMWF Grib Table */
extern struct gribtable_s ECMWF_gribtable[];
/** DWD1 Grib Table */
extern struct gribtable_s DWD1_gribtable[];
/** Local Grib Table */
extern struct gribtable_s local_gribtable[];
/** User-defined Grib Table */
extern struct gribtable_s *user_gribtable;

static struct gribtable_s *search_gribtable(struct gribtable_s *gribtable, unsigned char **sec);

#ifdef USE_TIGGE
/** Flag to use TIGGE */
extern int tigge;
/** TIGGE Grib Table */
extern struct gribtable_s tigge_gribtable[];
#endif
/*
 * get the name information    2006 Public Domain  Wesley Ebisuzaki
 *
 * if inv_out, name, desc, unit == NULL, not used

 */

/** Indicates the GRIB table name to use */
extern int names;

/**
 * Get the name information. 
 * 
 * @param sec Pointer to the GRIB section.
 * @param mode Mode of operation (0 for normal, -1 for initialization).
 * @param inv_out Pointer to the output string for the name. Ignored if NULL.
 * @param name Pointer to store the name. Ignored if NULL.
 * @param desc Pointer to store the description. Ignored if NULL.
 * @param unit Pointer to store the unit. Ignored if NULL.
 * @param mset Pointer to store the master table version number used by set_var.
 * @param mlow Pointer to store the low range of master tables.
 * @param mhigh Pointer to store the high range of master tables.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int getName_all(unsigned char **sec, int mode, char *inv_out, char *name, char *desc, char *unit, int *mset, int *mlow, int *mhigh) {

    int discipline, center, mastertab, localtab, parmcat, parmnum;
    int pdt, use_local_table;
    struct gribtable_s *p;
    const char *p_unit;

    p = NULL;

    /* check USER grib table first */
    if (user_gribtable != NULL) p = search_gribtable(user_gribtable, sec);

    /* use either NCEP or ECMWF tables */
    /* TIGGE is consider an extension of NCEP */
    /* if NCEP, include TIGGE defintions as before, otherwise ignore TIGGE */

    if (p == NULL && names == ECMWF) p = search_gribtable(ECMWF_gribtable, sec);
    if (p == NULL && names == DWD1) p = search_gribtable(DWD1_gribtable, sec);
    if (p == NULL && names == NCEP) {
#ifdef USE_TIGGE
        if (tigge && p == NULL) p = search_gribtable(tigge_gribtable, sec);		/* tigge is default table */
#endif
        if (p == NULL) p = search_gribtable(NCEP_gribtable, sec);
#ifdef USE_TIGGE
        /* if undefined and a tigge file */
        if (p == NULL && !tigge && (code_table_1_3(sec) == 4 || code_table_1_3(sec) == 5)) p = search_gribtable(tigge_gribtable, sec);
#endif
    }

    /* check local tables */
    if (p == NULL) {
        discipline = GB2_Discipline(sec);
        center = GB2_Center(sec);
        mastertab = GB2_MasterTable(sec);
        localtab = GB2_LocalTable(sec);
        parmcat = GB2_ParmCat(sec);
        parmnum = GB2_ParmNum(sec);
        use_local_table = (mastertab == 255) ? 1 : 0;
        if ((parmnum >= 192 && parmnum <= 254) || (parmcat >= 192 && parmcat <= 254)
            || (discipline >= 192 && discipline <= 254) ) use_local_table = 1;

        if (use_local_table) {
            if (center == ECMWF && names != ECMWF) p = search_gribtable(ECMWF_gribtable, sec);
            else if (center == DWD1 && names != DWD1) p = search_gribtable(DWD1_gribtable, sec);
            else if (center == DWD2 && names != DWD1) p = search_gribtable(DWD1_gribtable, sec);
            else if (center == NCEP && names != NCEP) p = search_gribtable(NCEP_gribtable, sec);
            else p = search_gribtable(local_gribtable, sec);
        }
    }

    p_unit = "unit";
    if (p) {
        p_unit = p->unit;
        pdt = code_table_4_0(sec);
        if (pdt == 5 || pdt == 9) p_unit = "prob";
    }

    if (p) {
        if (name) strcpy(name, p->name);
        if (desc) strcpy(desc, p->desc);
        if (unit) strcpy(unit, p_unit);

        if (inv_out) {
            sprintf(inv_out, "%s", p->name);
            inv_out += strlen(inv_out);
            if (mode) sprintf(inv_out," %s [%s]", p->desc, p_unit);
        }
        *mset = p->mtab_set;
        *mlow = p->mtab_low;
        *mhigh = p->mtab_high;
    }
    else {
        discipline = GB2_Discipline(sec);
        center = GB2_Center(sec);
        mastertab = GB2_MasterTable(sec);
        localtab = GB2_LocalTable(sec);
        parmcat = GB2_ParmCat(sec);
        parmnum = GB2_ParmNum(sec);
        *mset = 0;
        *mlow = 0;
        *mhigh = 255;

        if (name) sprintf(name,"var%d_%d_%d",discipline,parmcat,parmnum);
        if (desc) strcpy(desc,"desc");
        if (unit) strcpy(unit,p_unit);

        if (inv_out) {
            if ((parmnum >= 192 && parmnum <= 254) || (parmcat >= 192 && parmcat <= 254)
                || (discipline >= 192 && discipline <= 254) ) {
                sprintf(inv_out,"var discipline=%d center=%d local_table=%d parmcat=%d parm=%d",
                    discipline, center, localtab, parmcat, parmnum);
            }
            else {
                sprintf(inv_out,"var discipline=%d master_table=%d parmcat=%d parm=%d", 
                    discipline, mastertab, parmcat, parmnum);
            }
        }
    }

    return 0;
}

/**
 * Get the name information. 
 * 
 * @param sec Pointer to the GRIB section.
 * @param mode Mode of operation (0 for normal, -1 for initialization).
 * @param inv_out Pointer to the output string for the name. Ignored if NULL.
 * @param name Pointer to store the name. Ignored if NULL.
 * @param desc Pointer to store the description. Ignored if NULL.
 * @param unit Pointer to store the unit. Ignored if NULL.
 * 
 * @return 0 for success, error code otherwise.
 * 
 * @author Wesley Ebisuzaki @date 2006
 */
int getName(unsigned char **sec, int mode, char *inv_out, char *name, char *desc, char *unit) {
    int mset, mlow, mhigh;

    return getName_all(sec, mode, inv_out, name, desc, unit, &mset, &mlow, &mhigh);
}

 /**
  * Searches the GRIB table for a matching entry based on the GRIB section.
  * 
  * @param p Pointer to the start of the GRIB table.
  * @param sec Pointer to the GRIB section.
  * 
  * @return Pointer to the matching entry in the GRIB table, or NULL if not found.
  * 
  * @author Wesley Ebisuzaki @date 2006
  */
static struct gribtable_s *search_gribtable(struct gribtable_s *p, unsigned char **sec) {

    int discipline, center, mastertab, localtab, parmcat, parmnum;
    int use_local_table;
    static int count = 0;
    struct gribtable_s *p_orig;

    if (p == NULL) return NULL;

    discipline = GB2_Discipline(sec);
    center = GB2_Center(sec);
    mastertab = GB2_MasterTable(sec);
    localtab = GB2_LocalTable(sec);
    parmcat = GB2_ParmCat(sec);
    parmnum = GB2_ParmNum(sec);

    use_local_table = (mastertab == 255) ? 1 : 0;
    if ((parmnum >= 192 && parmnum <= 254) || (parmcat >= 192 && parmcat <= 254)
        || (discipline >= 192 && discipline <= 254) ) use_local_table = 1;
   
    if (use_local_table == 1 && localtab == 0) {
        if (count++ < 6) fprintf(stderr,"**** WARNING: local table = 0 is not allowed, will try fallback value 1 as well ***\n");
    }
    if (use_local_table == 1 && localtab == 255) {
        fatal_error("local gribtable is undefined (255)","");
    }

    if (! use_local_table) {
        for (; p->disc >= 0; p++) {
            if (discipline == p->disc && (mastertab >= p->mtab_low) && (mastertab <= p->mtab_high) &&
                    parmcat == p->pcat && parmnum == p->pnum) {
                return p;
            }
        }
    }
    else {
//	printf(">> cname local find: disc %d center %d localtab %d pcat %d pnum %d\n", discipline, center, localtab, parmcat, parmnum);
        p_orig = p;
    /* look for entry with original localtab value as derived from grib message */
        for (; p->disc >= 0; p++) {
            if (discipline == p->disc && center == p->cntr && localtab == p->ltab && 
                    parmcat == p->pcat && parmnum == p->pnum) {
                return p;
        }
    }
    /* fallback using localtab = 1 */
    for (; p_orig->disc >= 0; p_orig++) {
            if (discipline == p_orig->disc && center == p_orig->cntr && 1 == p_orig->ltab &&
                    parmcat == p_orig->pcat && parmnum == p_orig->pnum) {
                return p_orig;
            }
        }
    }
    return NULL;
}
