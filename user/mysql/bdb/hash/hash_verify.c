/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 *
 * $Id: hash_verify.c,v 1.31 2000/11/30 00:58:37 ubell Exp $
 */

#include "db_config.h"

#ifndef lint
static const char revid[] = "$Id: hash_verify.c,v 1.31 2000/11/30 00:58:37 ubell Exp $";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "db_verify.h"
#include "btree.h"
#include "hash.h"

static int __ham_dups_unsorted __P((DB *, u_int8_t *, u_int32_t));
static int __ham_vrfy_bucket __P((DB *, VRFY_DBINFO *, HMETA *, u_int32_t,
    u_int32_t));
static int __ham_vrfy_item __P((DB *,
    VRFY_DBINFO *, db_pgno_t, PAGE *, u_int32_t, u_int32_t));

/*
 * __ham_vrfy_meta --
 *	Verify the hash-specific part of a metadata page.
 *
 *	Note that unlike btree, we don't save things off, because we
 *	will need most everything again to verify each page and the
 *	amount of state here is significant.
 *
 * PUBLIC: int __ham_vrfy_meta __P((DB *, VRFY_DBINFO *, HMETA *,
 * PUBLIC:     db_pgno_t, u_int32_t));
 */
int
__ham_vrfy_meta(dbp, vdp, m, pgno, flags)
	DB *dbp;
	VRFY_DBINFO *vdp;
	HMETA *m;
	db_pgno_t pgno;
	u_int32_t flags;
{
	HASH *hashp;
	VRFY_PAGEINFO *pip;
	int i, ret, t_ret, isbad;
	u_int32_t pwr, mbucket;
	u_int32_t (*hfunc) __P((DB *, const void *, u_int32_t));

	if ((ret = __db_vrfy_getpageinfo(vdp, pgno, &pip)) != 0)
		return (ret);
	isbad = 0;

	hashp = dbp->h_internal;

	if (hashp != NULL && hashp->h_hash != NULL)
		hfunc = hashp->h_hash;
	else
		hfunc = __ham_func5;

	/*
	 * If we haven't already checked the common fields in pagezero,
	 * check them.
	 */
	if (!F_ISSET(pip, VRFY_INCOMPLETE) &&
	    (ret = __db_vrfy_meta(dbp, vdp, &m->dbmeta, pgno, flags)) != 0) {
		if (ret == DB_VERIFY_BAD)
			isbad = 1;
		else
			goto err;
	}

	/* h_charkey */
	if (!LF_ISSET(DB_NOORDERCHK))
		if (m->h_charkey != hfunc(dbp, CHARKEY, sizeof(CHARKEY))) {
			EPRINT((dbp->dbenv,
"Database has different custom hash function; reverify with DB_NOORDERCHK set"
			    ));
			/*
			 * Return immediately;  this is probably a sign
			 * of user error rather than database corruption, so
			 * we want to avoid extraneous errors.
			 */
			isbad = 1;
			goto err;
		}

	/* max_bucket must be less than the last pgno. */
	if (m->max_bucket > vdp->last_pgno) {
		EPRINT((dbp->dbenv,
		    "Impossible max_bucket %lu on meta page %lu",
		    m->max_bucket, pgno));
		/*
		 * Most other fields depend somehow on max_bucket, so
		 * we just return--there will be lots of extraneous
		 * errors.
		 */
		isbad = 1;
		goto err;
	}

	/*
	 * max_bucket, high_mask and low_mask: high_mask must be one
	 * less than the next power of two above max_bucket, and
	 * low_mask must be one less than the power of two below it.
	 *
	 *
	 */
	pwr = (m->max_bucket == 0) ? 1 : 1 << __db_log2(m->max_bucket + 1);
	if (m->high_mask != pwr - 1) {
		EPRINT((dbp->dbenv,
		    "Incorrect high_mask %lu on page %lu, should be %lu",
		    m->high_mask, pgno, pwr - 1));
		isbad = 1;
	}
	pwr >>= 1;
	if (m->low_mask != pwr - 1) {
		EPRINT((dbp->dbenv,
		    "Incorrect low_mask %lu on page %lu, should be %lu",
		    m->low_mask, pgno, pwr - 1));
		isbad = 1;
	}

	/* ffactor: no check possible. */
	pip->h_ffactor = m->ffactor;

	/*
	 * nelem: just make sure it's not astronomical for now. This is the
	 * same check that hash_upgrade does, since there was a bug in 2.X
	 * which could make nelem go "negative".
	 */
	if (m->nelem > 0x80000000) {
		EPRINT((dbp->dbenv,
		    "Suspiciously high nelem of %lu on page %lu",
		    m->nelem, pgno));
		isbad = 1;
		pip->h_nelem = 0;
	} else
		pip->h_nelem = m->nelem;

	/* flags */
	if (F_ISSET(&m->dbmeta, DB_HASH_DUP))
		F_SET(pip, VRFY_HAS_DUPS);
	if (F_ISSET(&m->dbmeta, DB_HASH_DUPSORT))
		F_SET(pip, VRFY_HAS_DUPSORT);
	/* XXX: Why is the DB_HASH_SUBDB flag necessary? */

	/* spares array */
	for (i = 0; m->spares[i] != 0 && i < NCACHED; i++) {
		/*
		 * We set mbucket to the maximum bucket that would use a given
		 * spares entry;  we want to ensure that it's always less
		 * than last_pgno.
		 */
		mbucket = (1 << i) - 1;
		if (BS_TO_PAGE(mbucket, m->spares) > vdp->last_pgno) {
			EPRINT((dbp->dbenv,
			    "Spares array entry %lu, page %lu is invalid",
			    i, pgno));
			isbad = 1;
		}
	}

err:	if ((t_ret = __db_vrfy_putpageinfo(vdp, pip)) != 0 && ret == 0)
		ret = t_ret;
	return ((ret == 0 && isbad == 1) ? DB_VERIFY_BAD : ret);
}

/*
 * __ham_vrfy --
 *	Verify hash page.
 *
 * PUBLIC: int __ham_vrfy __P((DB *, VRFY_DBINFO *, PAGE *, db_pgno_t,
 * PUBLIC:     u_int32_t));
 */
int
__ham_vrfy(dbp, vdp, h, pgno, flags)
	DB *dbp;
	VRFY_DBINFO *vdp;
	PAGE *h;
	db_pgno_t pgno;
	u_int32_t flags;
{
	VRFY_PAGEINFO *pip;
	u_int32_t ent, himark, inpend;
	int isbad, ret, t_ret;

	isbad = 0;
	if ((ret = __db_vrfy_getpageinfo(vdp, pgno, &pip)) != 0)
		return (ret);

	/* Sanity check our flags and page type. */
	if ((ret = __db_fchk(dbp->dbenv, "__ham_vrfy",
	    flags, DB_AGGRESSIVE | DB_NOORDERCHK | DB_SALVAGE)) != 0)
		goto err;

	if (TYPE(h) != P_HASH) {
		TYPE_ERR_PRINT(dbp->dbenv, "__ham_vrfy", pgno, TYPE(h));
		DB_ASSERT(0);
		ret = EINVAL;
		goto err;
	}

	/* Verify and save off fields common to all PAGEs. */
	if ((ret = __db_vrfy_datapage(dbp, vdp, h, pgno, flags)) != 0) {
		if (ret == DB_VERIFY_BAD)
			isbad = 1;
		else
			goto err;
	}

	/*
	 * Verify inp[].  Each offset from 0 to NUM_ENT(h) must be lower
	 * than the previous one, higher than the current end of the inp array,
	 * and lower than the page size.
	 *
	 * In any case, we return immediately if things are bad, as it would
	 * be unsafe to proceed.
	 */
	for (ent = 0, himark = dbp->pgsize,
	    inpend = (u_int8_t *)h->inp - (u_int8_t *)h;
	    ent < NUM_ENT(h); ent++)
		if (h->inp[ent] >= himark) {
			EPRINT((dbp->dbenv,
			    "Item %lu on page %lu out of order or nonsensical",
			    ent, pgno));
			isbad = 1;
			goto err;
		} else if (inpend >= himark) {
			EPRINT((dbp->dbenv,
			    "inp array collided with data on page %lu",
			    pgno));
			isbad = 1;
			goto err;

		} else {
			himark = h->inp[ent];
			inpend += sizeof(db_indx_t);
			if ((ret = __ham_vrfy_item(
			    dbp, vdp, pgno, h, ent, flags)) != 0)
				goto err;
		}

err:	if ((t_ret = __db_vrfy_putpageinfo(vdp, pip)) != 0 && ret == 0)
		ret = t_ret;
	return (ret == 0 && isbad == 1 ? DB_VERIFY_BAD : ret);
}

/*
 * __ham_vrfy_item --
 *	Given a hash page and an offset, sanity-check the item itself,
 *	and save off any overflow items or off-page dup children as necessary.
 */
static int
__ham_vrfy_item(dbp, vdp, pgno, h, i, flags)
	DB *dbp;
	VRFY_DBINFO *vdp;
	db_pgno_t pgno;
	PAGE *h;
	u_int32_t i, flags;
{
	HOFFPAGE hop;
	HOFFDUP hod;
	VRFY_CHILDINFO child;
	VRFY_PAGEINFO *pip;
	db_indx_t offset, len, dlen, elen;
	int ret, t_ret;
	u_int8_t *databuf;

	if ((ret = __db_vrfy_getpageinfo(vdp, pgno, &pip)) != 0)
		return (ret);

	switch (HPAGE_TYPE(h, i)) {
	case H_KEYDATA:
		/* Nothing to do here--everything but the type field is data */
		break;
	case H_DUPLICATE:
		/* Are we a datum or a key?  Better be the former. */
		if (i % 2 == 0) {
			EPRINT((dbp->dbenv,
			    "Hash key stored as duplicate at page %lu item %lu",
			    pip->pgno, i));
		}
		/*
		 * Dups are encoded as a series within a single HKEYDATA,
		 * in which each dup is surrounded by a copy of its length
		 * on either side (so that the series can be walked in either
		 * direction.  We loop through this series and make sure
		 * each dup is reasonable.
		 *
		 * Note that at this point, we've verified item i-1, so
		 * it's safe to use LEN_HKEYDATA (which looks at inp[i-1]).
		 */
		len = LEN_HKEYDATA(h, dbp->pgsize, i);
		databuf = HKEYDATA_DATA(P_ENTRY(h, i));
		for (offset = 0; offset < len; offset += DUP_SIZE(dlen)) {
			memcpy(&dlen, databuf + offset, sizeof(db_indx_t));

			/* Make sure the length is plausible. */
			if (offset + DUP_SIZE(dlen) > len) {
				EPRINT((dbp->dbenv,
			    "Duplicate item %lu, page %lu has bad length",
				    i, pip->pgno));
				ret = DB_VERIFY_BAD;
				goto err;
			}

			/*
			 * Make sure the second copy of the length is the
			 * same as the first.
			 */
			memcpy(&elen,
			    databuf + offset + dlen + sizeof(db_indx_t),
			    sizeof(db_indx_t));
			if (elen != dlen) {
				EPRINT((dbp->dbenv,
		"Duplicate item %lu, page %lu has two different lengths",
				    i, pip->pgno));
				ret = DB_VERIFY_BAD;
				goto err;
			}
		}
		F_SET(pip, VRFY_HAS_DUPS);
		if (!LF_ISSET(DB_NOORDERCHK) &&
		    __ham_dups_unsorted(dbp, databuf, len))
			F_SET(pip, VRFY_DUPS_UNSORTED);
		break;
	case H_OFFPAGE:
		/* Offpage item.  Make sure pgno is sane, save off. */
		memcpy(&hop, P_ENTRY(h, i), HOFFPAGE_SIZE);
		if (!IS_VALID_PGNO(hop.pgno) || hop.pgno == pip->pgno ||
		    hop.pgno == PGNO_INVALID) {
			EPRINT((dbp->dbenv,
			    "Offpage item %lu, page %lu has bad page number",
			    i, pip->pgno));
			ret = DB_VERIFY_BAD;
			goto err;
		}
		memset(&child, 0, sizeof(VRFY_CHILDINFO));
		child.pgno = hop.pgno;
		child.type = V_OVERFLOW;
		child.tlen = hop.tlen; /* This will get checked later. */
		if ((ret = __db_vrfy_childput(vdp, pip->pgno, &child)) != 0)
			goto err;
		break;
	case H_OFFDUP:
		/* Offpage duplicate item.  Same drill. */
		memcpy(&hod, P_ENTRY(h, i), HOFFDUP_SIZE);
		if (!IS_VALID_PGNO(hod.pgno) || hod.pgno == pip->pgno ||
		    hod.pgno == PGNO_INVALID) {
			EPRINT((dbp->dbenv,
			    "Offpage item %lu, page %lu has bad page number",
			    i, pip->pgno));
			ret = DB_VERIFY_BAD;
			goto err;
		}
		memset(&child, 0, sizeof(VRFY_CHILDINFO));
		child.pgno = hod.pgno;
		child.type = V_DUPLICATE;
		if ((ret = __db_vrfy_childput(vdp, pip->pgno, &child)) != 0)
			goto err;
		F_SET(pip, VRFY_HAS_DUPS);
		break;
	default:
		EPRINT((dbp->dbenv,
		    "Item %i, page %lu has bad type", i, pip->pgno));
		ret = DB_VERIFY_BAD;
		break;
	}

err:	if ((t_ret = __db_vrfy_putpageinfo(vdp, pip)) != 0 && ret == 0)
		ret = t_ret;
	return (ret);
}

/*
 * __ham_vrfy_structure --
 *	Verify the structure of a hash database.
 *
 * PUBLIC: int __ham_vrfy_structure __P((DB *, VRFY_DBINFO *, db_pgno_t,
 * PUBLIC:     u_int32_t));
 */
int
__ham_vrfy_structure(dbp, vdp, meta_pgno, flags)
	DB *dbp;
	VRFY_DBINFO *vdp;
	db_pgno_t meta_pgno;
	u_int32_t flags;
{
	DB *pgset;
	HMETA *m;
	PAGE *h;
	VRFY_PAGEINFO *pip;
	int isbad, p, ret, t_ret;
	db_pgno_t pgno;
	u_int32_t bucket;

	ret = isbad = 0;
	h = NULL;
	pgset = vdp->pgset;

	if ((ret = __db_vrfy_pgset_get(pgset, meta_pgno, &p)) != 0)
		return (ret);
	if (p != 0) {
		EPRINT((dbp->dbenv,
		    "Hash meta page %lu referenced twice", meta_pgno));
		return (DB_VERIFY_BAD);
	}
	if ((ret = __db_vrfy_pgset_inc(pgset, meta_pgno)) != 0)
		return (ret);

	/* Get the meta page;  we'll need it frequently. */
	if ((ret = memp_fget(dbp->mpf, &meta_pgno, 0, &m)) != 0)
		return (ret);

	/* Loop through bucket by bucket. */
	for (bucket = 0; bucket <= m->max_bucket; bucket++)
		if ((ret =
		    __ham_vrfy_bucket(dbp, vdp, m, bucket, flags)) != 0) {
			if (ret == DB_VERIFY_BAD)
				isbad = 1;
			else
				goto err;
		    }

	/*
	 * There may be unused hash pages corresponding to buckets
	 * that have been allocated but not yet used.  These may be
	 * part of the current doubling above max_bucket, or they may
	 * correspond to buckets that were used in a transaction
	 * that then aborted.
	 *
	 * Loop through them, as far as the spares array defines them,
	 * and make sure they're all empty.
	 *
	 * Note that this should be safe, since we've already verified
	 * that the spares array is sane.
	 */
	for (bucket = m->max_bucket + 1;
	    m->spares[__db_log2(bucket + 1)] != 0; bucket++) {
		pgno = BS_TO_PAGE(bucket, m->spares);
		if ((ret = __db_vrfy_getpageinfo(vdp, pgno, &pip)) != 0)
			goto err;

		/* It's okay if these pages are totally zeroed;  unmark it. */
		F_CLR(pip, VRFY_IS_ALLZEROES);

		if (pip->type != P_HASH) {
			EPRINT((dbp->dbenv,
			    "Hash bucket %lu maps to non-hash page %lu",
			    bucket, pgno));
			isbad = 1;
		} else if (pip->entries != 0) {
			EPRINT((dbp->dbenv,
			    "Non-empty page %lu in unused hash bucket %lu",
			    pgno, bucket));
			isbad = 1;
		} else {
			if ((ret = __db_vrfy_pgset_get(pgset, pgno, &p)) != 0)
				goto err;
			if (p != 0) {
				EPRINT((dbp->dbenv,
				    "Hash page %lu above max_bucket referenced",
				    pgno));
				isbad = 1;
			} else {
				if ((ret =
				    __db_vrfy_pgset_inc(pgset, pgno)) != 0)
					goto err;
				if ((ret =
				    __db_vrfy_putpageinfo(vdp, pip)) != 0)
					goto err;
				continue;
			}
		}

		/* If we got here, it's an error. */
		(void)__db_vrfy_putpageinfo(vdp, pip);
		goto err;
	}

err:	if ((t_ret = memp_fput(dbp->mpf, m, 0)) != 0)
		return (t_ret);
	if (h != NULL && (t_ret = memp_fput(dbp->mpf, h, 0)) != 0)
		return (t_ret);
	return ((isbad == 1 && ret == 0) ? DB_VERIFY_BAD: ret);
}

/*
 * __ham_vrfy_bucket --
 *	Verify a given bucket.
 */
static int
__ham_vrfy_bucket(dbp, vdp, m, bucket, flags)
	DB *dbp;
	VRFY_DBINFO *vdp;
	HMETA *m;
	u_int32_t bucket, flags;
{
	HASH *hashp;
	VRFY_CHILDINFO *child;
	VRFY_PAGEINFO *mip, *pip;
	int ret, t_ret, isbad, p;
	db_pgno_t pgno, next_pgno;
	DBC *cc;
	u_int32_t (*hfunc) __P((DB *, const void *, u_int32_t));

	isbad = 0;
	pip = NULL;
	cc = NULL;

	hashp = dbp->h_internal;
	if (hashp != NULL && hashp->h_hash != NULL)
		hfunc = hashp->h_hash;
	else
		hfunc = __ham_func5;

	if ((ret = __db_vrfy_getpageinfo(vdp, PGNO(m), &mip)) != 0)
		return (ret);

	/* Calculate the first pgno for this bucket. */
	pgno = BS_TO_PAGE(bucket, m->spares);

	if ((ret = __db_vrfy_getpageinfo(vdp, pgno, &pip)) != 0)
		goto err;

	/* Make sure we got a plausible page number. */
	if (pgno > vdp->last_pgno || pip->type != P_HASH) {
		EPRINT((dbp->dbenv, "Bucket %lu has impossible first page %lu",
		    bucket, pgno));
		/* Unsafe to continue. */
		isbad = 1;
		goto err;
	}

	if (pip->prev_pgno != PGNO_INVALID) {
		EPRINT((dbp->dbenv,
		    "First hash page %lu in bucket %lu has a prev_pgno", pgno));
		isbad = 1;
	}

	/*
	 * Set flags for dups and sorted dups.
	 */
	flags |= F_ISSET(mip, VRFY_HAS_DUPS) ? ST_DUPOK : 0;
	flags |= F_ISSET(mip, VRFY_HAS_DUPSORT) ? ST_DUPSORT : 0;

	/* Loop until we find a fatal bug, or until we run out of pages. */
	for (;;) {
		/* Provide feedback on our progress to the application. */
		if (!LF_ISSET(DB_SALVAGE))
			__db_vrfy_struct_feedback(dbp, vdp);

		if ((ret = __db_vrfy_pgset_get(vdp->pgset, pgno, &p)) != 0)
			goto err;
		if (p != 0) {
			EPRINT((dbp->dbenv,
			    "Hash page %lu referenced twice", pgno));
			isbad = 1;
			/* Unsafe to continue. */
			goto err;
		} else if ((ret = __db_vrfy_pgset_inc(vdp->pgset, pgno)) != 0)
			goto err;

		/*
		 * Hash pages that nothing has ever hashed to may never
		 * have actually come into existence, and may appear to be
		 * entirely zeroed.  This is acceptable, and since there's
		 * no real way for us to know whether this has actually
		 * occurred, we clear the "wholly zeroed" flag on every
		 * hash page.  A wholly zeroed page, by nature, will appear
		 * to have no flags set and zero entries, so should
		 * otherwise verify correctly.
		 */
		F_CLR(pip, VRFY_IS_ALLZEROES);

		/* If we have dups, our meta page had better know about it. */
		if (F_ISSET(pip, VRFY_HAS_DUPS)
		    && !F_ISSET(mip, VRFY_HAS_DUPS)) {
			EPRINT((dbp->dbenv,
		    "Duplicates present in non-duplicate database, page %lu",
			    pgno));
			isbad = 1;
		}

		/*
		 * If the database has sorted dups, this page had better
		 * not have unsorted ones.
		 */
		if (F_ISSET(mip, VRFY_HAS_DUPSORT) &&
		    F_ISSET(pip, VRFY_DUPS_UNSORTED)) {
			EPRINT((dbp->dbenv,
			    "Unsorted dups in sorted-dup database, page %lu",
			    pgno));
			isbad = 1;
		}

		/* Walk overflow chains and offpage dup trees. */
		if ((ret = __db_vrfy_childcursor(vdp, &cc)) != 0)
			goto err;
		for (ret = __db_vrfy_ccset(cc, pip->pgno, &child); ret == 0;
		    ret = __db_vrfy_ccnext(cc, &child))
			if (child->type == V_OVERFLOW) {
				if ((ret = __db_vrfy_ovfl_structure(dbp, vdp,
				    child->pgno, child->tlen, flags)) != 0) {
					if (ret == DB_VERIFY_BAD)
						isbad = 1;
					else
						goto err;
				}
			} else if (child->type == V_DUPLICATE) {
				if ((ret = __db_vrfy_duptype(dbp,
				    vdp, child->pgno, flags)) != 0) {
					isbad = 1;
					continue;
				}
				if ((ret = __bam_vrfy_subtree(dbp, vdp,
				    child->pgno, NULL, NULL,
				    flags | ST_RECNUM | ST_DUPSET, NULL,
				    NULL, NULL)) != 0) {
					if (ret == DB_VERIFY_BAD)
						isbad = 1;
					else
						goto err;
				}
			}
		if ((ret = __db_vrfy_ccclose(cc)) != 0)
			goto err;
		cc = NULL;

		/* If it's safe to check that things hash properly, do so. */
		if (isbad == 0 && !LF_ISSET(DB_NOORDERCHK) &&
		    (ret = __ham_vrfy_hashing(dbp, pip->entries,
		    m, bucket, pgno, flags, hfunc)) != 0) {
			if (ret == DB_VERIFY_BAD)
				isbad = 1;
			else
				goto err;
		}

		next_pgno = pip->next_pgno;
		ret = __db_vrfy_putpageinfo(vdp, pip);

		pip = NULL;
		if (ret != 0)
			goto err;

		if (next_pgno == PGNO_INVALID)
			break;		/* End of the bucket. */

		/* We already checked this, but just in case... */
		if (!IS_VALID_PGNO(next_pgno)) {
			DB_ASSERT(0);
			EPRINT((dbp->dbenv,
			    "Hash page %lu has bad next_pgno", pgno));
			isbad = 1;
			goto err;
		}

		if ((ret = __db_vrfy_getpageinfo(vdp, next_pgno, &pip)) != 0)
			goto err;

		if (pip->prev_pgno != pgno) {
			EPRINT((dbp->dbenv, "Hash page %lu has bad prev_pgno",
			    next_pgno));
			isbad = 1;
		}
		pgno = next_pgno;
	}

err:	if (cc != NULL && ((t_ret = __db_vrfy_ccclose(cc)) != 0) && ret == 0)
		ret = t_ret;
	if (mip != NULL && ((t_ret = __db_vrfy_putpageinfo(vdp, mip)) != 0) &&
	    ret == 0)
		ret = t_ret;
	if (pip != NULL && ((t_ret = __db_vrfy_putpageinfo(vdp, pip)) != 0) &&
	    ret == 0)
		ret = t_ret;
	return ((ret == 0 && isbad == 1) ? DB_VERIFY_BAD : ret);
}

/*
 * __ham_vrfy_hashing --
 *	Verify that all items on a given hash page hash correctly.
 *
 * PUBLIC: int __ham_vrfy_hashing __P((DB *,
 * PUBLIC:     u_int32_t, HMETA *, u_int32_t, db_pgno_t, u_int32_t,
 * PUBLIC:     u_int32_t (*) __P((DB *, const void *, u_int32_t))));
 */
int
__ham_vrfy_hashing(dbp, nentries, m, thisbucket, pgno, flags, hfunc)
	DB *dbp;
	u_int32_t nentries;
	HMETA *m;
	u_int32_t thisbucket;
	db_pgno_t pgno;
	u_int32_t flags;
	u_int32_t (*hfunc) __P((DB *, const void *, u_int32_t));
{
	DBT dbt;
	PAGE *h;
	db_indx_t i;
	int ret, t_ret, isbad;
	u_int32_t hval, bucket;

	ret = isbad = 0;
	memset(&dbt, 0, sizeof(DBT));
	F_SET(&dbt, DB_DBT_REALLOC);

	if ((ret = memp_fget(dbp->mpf, &pgno, 0, &h)) != 0)
		return (ret);

	for (i = 0; i < nentries; i += 2) {
		/*
		 * We've already verified the page integrity and that of any
		 * overflow chains linked off it;  it is therefore safe to use
		 * __db_ret.  It's also not all that much slower, since we have
		 * to copy every hash item to deal with alignment anyway;  we
		 * can tweak this a bit if this proves to be a bottleneck,
		 * but for now, take the easy route.
		 */
		if ((ret = __db_ret(dbp, h, i, &dbt, NULL, NULL)) != 0)
			goto err;
		hval = hfunc(dbp, dbt.data, dbt.size);

		bucket = hval & m->high_mask;
		if (bucket > m->max_bucket)
			bucket = bucket & m->low_mask;

		if (bucket != thisbucket) {
			EPRINT((dbp->dbenv,
			    "Item %lu on page %lu hashes incorrectly",
			    i, pgno));
			isbad = 1;
		}
	}

err:	if (dbt.data != NULL)
		__os_free(dbt.data, 0);
	if ((t_ret = memp_fput(dbp->mpf, h, 0)) != 0)
		return (t_ret);

	return ((ret == 0 && isbad == 1) ? DB_VERIFY_BAD : ret);
}

/*
 * __ham_salvage --
 *	Safely dump out anything that looks like a key on an alleged
 *	hash page.
 *
 * PUBLIC: int __ham_salvage __P((DB *, VRFY_DBINFO *, db_pgno_t, PAGE *,
 * PUBLIC:     void *, int (*)(void *, const void *), u_int32_t));
 */
int
__ham_salvage(dbp, vdp, pgno, h, handle, callback, flags)
	DB *dbp;
	VRFY_DBINFO *vdp;
	db_pgno_t pgno;
	PAGE *h;
	void *handle;
	int (*callback) __P((void *, const void *));
	u_int32_t flags;
{
	DBT dbt, unkdbt;
	db_pgno_t dpgno;
	int ret, err_ret, t_ret;
	u_int32_t himark, tlen;
	u_int8_t *hk;
	void *buf;
	u_int32_t dlen, len, i;

	memset(&dbt, 0, sizeof(DBT));
	dbt.flags = DB_DBT_REALLOC;

	memset(&unkdbt, 0, sizeof(DBT));
	unkdbt.size = strlen("UNKNOWN") + 1;
	unkdbt.data = "UNKNOWN";

	err_ret = 0;

	/*
	 * Allocate a buffer for overflow items.  Start at one page;
	 * __db_safe_goff will realloc as needed.
	 */
	if ((ret = __os_malloc(dbp->dbenv, dbp->pgsize, NULL, &buf)) != 0)
		return (ret);

	himark = dbp->pgsize;
	for (i = 0;; i++) {
		/* If we're not aggressive, break when we hit NUM_ENT(h). */
		if (!LF_ISSET(DB_AGGRESSIVE) && i >= NUM_ENT(h))
			break;

		/* Verify the current item. */
		ret = __db_vrfy_inpitem(dbp,
		    h, pgno, i, 0, flags, &himark, NULL);
		/* If this returned a fatality, it's time to break. */
		if (ret == DB_VERIFY_FATAL)
			break;

		if (ret == 0) {
			hk = P_ENTRY(h, i);
			len = LEN_HKEYDATA(h, dbp->pgsize, i);
			if ((u_int32_t)(hk + len - (u_int8_t *)h) >
			    dbp->pgsize) {
				/*
				 * Item is unsafely large;  either continue
				 * or set it to the whole page, depending on
				 * aggressiveness.
				 */
				if (!LF_ISSET(DB_AGGRESSIVE))
					continue;
				len = dbp->pgsize -
				    (u_int32_t)(hk - (u_int8_t *)h);
				err_ret = DB_VERIFY_BAD;
			}
			switch (HPAGE_PTYPE(hk)) {
			default:
				if (!LF_ISSET(DB_AGGRESSIVE))
					break;
				err_ret = DB_VERIFY_BAD;
				/* FALLTHROUGH */
			case H_KEYDATA:
keydata:			memcpy(buf, HKEYDATA_DATA(hk), len);
				dbt.size = len;
				dbt.data = buf;
				if ((ret = __db_prdbt(&dbt,
				    0, " ", handle, callback, 0, NULL)) != 0)
					err_ret = ret;
				break;
			case H_OFFPAGE:
				if (len < HOFFPAGE_SIZE) {
					err_ret = DB_VERIFY_BAD;
					continue;
				}
				memcpy(&dpgno,
				    HOFFPAGE_PGNO(hk), sizeof(dpgno));
				if ((ret = __db_safe_goff(dbp, vdp,
				    dpgno, &dbt, &buf, flags)) != 0) {
					err_ret = ret;
					(void)__db_prdbt(&unkdbt, 0, " ",
					    handle, callback, 0, NULL);
					break;
				}
				if ((ret = __db_prdbt(&dbt,
				    0, " ", handle, callback, 0, NULL)) != 0)
					err_ret = ret;
				break;
			case H_OFFDUP:
				if (len < HOFFPAGE_SIZE) {
					err_ret = DB_VERIFY_BAD;
					continue;
				}
				memcpy(&dpgno,
				    HOFFPAGE_PGNO(hk), sizeof(dpgno));
				/* UNKNOWN iff pgno is bad or we're a key. */
				if (!IS_VALID_PGNO(dpgno) || (i % 2 == 0)) {
					if ((ret = __db_prdbt(&unkdbt, 0, " ",
					    handle, callback, 0, NULL)) != 0)
						err_ret = ret;
				} else if ((ret = __db_salvage_duptree(dbp,
				    vdp, dpgno, &dbt, handle, callback,
				    flags | SA_SKIPFIRSTKEY)) != 0)
					err_ret = ret;
				break;
			case H_DUPLICATE:
				/*
				 * We're a key;  printing dups will seriously
				 * foul the output.  If we're being aggressive,
				 * pretend this is a key and let the app.
				 * programmer sort out the mess.
				 */
				if (i % 2 == 0) {
					err_ret = ret;
					if (LF_ISSET(DB_AGGRESSIVE))
						goto keydata;
					break;
				}

				/* Too small to have any data. */
				if (len <
				    HKEYDATA_SIZE(2 * sizeof(db_indx_t))) {
					err_ret = DB_VERIFY_BAD;
					continue;
				}

				/* Loop until we hit the total length. */
				for (tlen = 0; tlen + sizeof(db_indx_t) < len;
				    tlen += dlen) {
					tlen += sizeof(db_indx_t);
					memcpy(&dlen, hk, sizeof(db_indx_t));
					/*
					 * If dlen is too long, print all the
					 * rest of the dup set in a chunk.
					 */
					if (dlen + tlen > len)
						dlen = len - tlen;
					memcpy(buf, hk + tlen, dlen);
					dbt.size = dlen;
					dbt.data = buf;
					if ((ret = __db_prdbt(&dbt, 0, " ",
					    handle, callback, 0, NULL)) != 0)
						err_ret = ret;
					tlen += sizeof(db_indx_t);
				}
				break;
			}
		}
	}

	__os_free(buf, 0);
	if ((t_ret = __db_salvage_markdone(vdp, pgno)) != 0)
		return (t_ret);
	return ((ret == 0 && err_ret != 0) ? err_ret : ret);
}

/*
 * __ham_meta2pgset --
 *	Return the set of hash pages corresponding to the given
 *	known-good meta page.
 *
 * PUBLIC: int __ham_meta2pgset __P((DB *, VRFY_DBINFO *, HMETA *, u_int32_t,
 * PUBLIC:     DB *));
 */
int __ham_meta2pgset(dbp, vdp, hmeta, flags, pgset)
	DB *dbp;
	VRFY_DBINFO *vdp;
	HMETA *hmeta;
	u_int32_t flags;
	DB *pgset;
{
	PAGE *h;
	db_pgno_t pgno;
	u_int32_t bucket, totpgs;
	int ret, val;

	/*
	 * We don't really need flags, but leave them for consistency with
	 * __bam_meta2pgset.
	 */
	COMPQUIET(flags, 0);

	DB_ASSERT(pgset != NULL);

	totpgs = 0;

	/*
	 * Loop through all the buckets, pushing onto pgset the corresponding
	 * page(s) for each one.
	 */
	for (bucket = 0; bucket <= hmeta->max_bucket; bucket++) {
		pgno = BS_TO_PAGE(bucket, hmeta->spares);

		/*
		 * We know the initial pgno is safe because the spares array has
		 * been verified.
		 *
		 * Safely walk the list of pages in this bucket.
		 */
		for (;;) {
			if ((ret = memp_fget(dbp->mpf, &pgno, 0, &h)) != 0)
				return (ret);
			if (TYPE(h) == P_HASH) {

				/*
				 * Make sure we don't go past the end of
				 * pgset.
				 */
				if (++totpgs > vdp->last_pgno) {
					(void)memp_fput(dbp->mpf, h, 0);
					return (DB_VERIFY_BAD);
				}
				if ((ret =
				    __db_vrfy_pgset_inc(pgset, pgno)) != 0)
					return (ret);

				pgno = NEXT_PGNO(h);
			} else
				pgno = PGNO_INVALID;

			if ((ret = memp_fput(dbp->mpf, h, 0)) != 0)
				return (ret);

			/* If the new pgno is wonky, go onto the next bucket. */
			if (!IS_VALID_PGNO(pgno) ||
			    pgno == PGNO_INVALID)
				goto nextbucket;

			/*
			 * If we've touched this page before, we have a cycle;
			 * go on to the next bucket.
			 */
			if ((ret = __db_vrfy_pgset_get(pgset, pgno, &val)) != 0)
				return (ret);
			if (val != 0)
				goto nextbucket;
		}
nextbucket:	;
	}
	return (0);
}

/*
 * __ham_dups_unsorted --
 *	Takes a known-safe hash duplicate set and its total length.
 *	Returns 1 if there are out-of-order duplicates in this set,
 *	0 if there are not.
 */
static int
__ham_dups_unsorted(dbp, buf, len)
	DB *dbp;
	u_int8_t *buf;
	u_int32_t len;
{
	DBT a, b;
	db_indx_t offset, dlen;
	int (*func) __P((DB *, const DBT *, const DBT *));

	memset(&a, 0, sizeof(DBT));
	memset(&b, 0, sizeof(DBT));

	func = (dbp->dup_compare == NULL) ? __bam_defcmp : dbp->dup_compare;

	/*
	 * Loop through the dup set until we hit the end or we find
	 * a pair of dups that's out of order.  b is always the current
	 * dup, a the one before it.
	 */
	for (offset = 0; offset < len; offset += DUP_SIZE(dlen)) {
		memcpy(&dlen, buf + offset, sizeof(db_indx_t));
		b.data = buf + offset + sizeof(db_indx_t);
		b.size = dlen;

		if (a.data != NULL && func(dbp, &a, &b) > 0)
			return (1);

		a.data = b.data;
		a.size = b.size;
	}

	return (0);
}
