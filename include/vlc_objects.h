/*****************************************************************************
 * vlc_objects.h: vlc_object_t definition and manipulation methods
 *****************************************************************************
 * Copyright (C) 2002-2008 VLC authors and VideoLAN
 *
 * Authors: Samuel Hocevar <sam@zoy.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/**
 * \defgroup vlc_object VLC objects
 * \ingroup vlc
 * @{
 * \file
 * Common VLC object defintions
 */

struct vlc_logger;

/**
 * VLC object common members
 *
 * Common public properties for all VLC objects.
 * Object also have private properties maintained by the core, see
 * \ref vlc_object_internals_t
 */
struct vlc_common_members
{
    struct vlc_logger *logger;

    bool no_interact;

    /** Module probe flag
     *
     * A boolean during module probing when the probe is "forced".
     * See \ref module_need().
     */
    bool force;
};

/**
 * Type-safe vlc_object_t cast
 *
 * This macro attempts to cast a pointer to a compound type to a
 * \ref vlc_object_t pointer in a type-safe manner.
 * It checks if the compound type actually starts with an embedded
 * \ref vlc_object_t structure.
 */
#if !defined(__cplusplus)
# define VLC_OBJECT(x) \
    _Generic((x)->obj, \
        struct vlc_common_members: (vlc_object_t *)(x) \
    )
#else
# define VLC_OBJECT(x) ((vlc_object_t *)(x))
#endif

/*****************************************************************************
 * The vlc_object_t type. Yes, it's that simple :-)
 *****************************************************************************/
/** The main vlc_object_t structure */
struct vlc_object_t
{
    struct vlc_common_members obj;
};

/* The root object */
struct libvlc_int_t
{
    struct vlc_common_members obj;
};

/*****************************************************************************
 * Prototypes
 *****************************************************************************/
VLC_API void *vlc_object_create( vlc_object_t *, size_t ) VLC_MALLOC VLC_USED;
VLC_API vlc_object_t *vlc_object_find_name( vlc_object_t *, const char * ) VLC_USED VLC_DEPRECATED;
VLC_API void * vlc_object_hold( vlc_object_t * );
VLC_API void vlc_object_release( vlc_object_t * );
VLC_API size_t vlc_list_children(vlc_object_t *, vlc_object_t **, size_t) VLC_USED;

/**
 * Returns the object type name.
 *
 * This returns a nul-terminated string identifying the object type.
 * The string is valid for at least as long as the object reference.
 *
 * \param obj object whose type name to get
 */
VLC_API const char *vlc_object_typename(const vlc_object_t *obj) VLC_USED;

/**
 * Gets the parent of an object.
 *
 * \return the parent object (NULL if none)
 *
 * \note The returned parent object pointer is valid as long as the child is.
 */
VLC_API vlc_object_t *vlc_object_parent(vlc_object_t *obj) VLC_USED;
#define vlc_object_parent(o) vlc_object_parent(VLC_OBJECT(o))

VLC_API char *vlc_object_get_name( const vlc_object_t * ) VLC_USED;

#define vlc_object_create(a,b) vlc_object_create( VLC_OBJECT(a), b )

#define vlc_object_find_name(a,b) \
    vlc_object_find_name( VLC_OBJECT(a),b)

#define vlc_object_hold(a) \
    vlc_object_hold( VLC_OBJECT(a) )

#define vlc_object_release(a) \
    vlc_object_release( VLC_OBJECT(a) )

VLC_USED
static inline libvlc_int_t *vlc_object_instance(vlc_object_t *obj)
{
    vlc_object_t *parent;

    do
        parent = obj;
    while ((obj = vlc_object_parent(obj)) != NULL);

    return (libvlc_int_t *)parent;
}
#define vlc_object_instance(o) vlc_object_instance(VLC_OBJECT(o))

/**
 * @defgroup objres Object resources
 *
 * The object resource functions tie resource allocation to an instance of
 * a module through a VLC object.
 * Such resource will be automatically freed, in first in last out order,
 * when the module instance associated with the VLC object is terminated.
 *
 * Specifically, if the module instance activation/probe function fails, the
 * resource will be freed immediately after the failure within
 * vlc_module_load(). If the activation succeeds, the resource will be freed
 * when the module instance is terminated with vlc_module_unload().
 *
 * This is a convenience mechanism to save explicit clean-up function calls
 * in modules.
 *
 * @{
 */

/**
 * Allocates memory for a module.
 *
 * This function allocates memory from the heap for a module instance.
 * The memory is uninitialized.
 *
 * @param obj VLC object to tie the memory allocation to
 * @param size byte size of the memory allocation
 *
 * @return a pointer to the allocated memory, or NULL on error (errno is set).
 */
VLC_API VLC_MALLOC void *vlc_obj_malloc(vlc_object_t *obj, size_t size);

/**
 * Allocates a zero-initialized table for a module.
 *
 * This function allocates a table from the heap for a module instance.
 * The memory is initialized to all zeroes.
 *
 * @param obj VLC object to tie the memory allocation to
 * @param nmemb number of table entries
 * @param size byte size of a table entry
 *
 * @return a pointer to the allocated memory, or NULL on error (errno is set).
 */
VLC_API VLC_MALLOC void *vlc_obj_calloc(vlc_object_t *obj, size_t nmemb,
                                        size_t size);

/**
 * Duplicates a string for a module.
 *
 * This function allocates a copy of a nul-terminated string for a module
 * instance.
 *
 * @param obj VLC object to tie the memory allocation to
 * @param str string to copy
 *
 * @return a pointer to the copy, or NULL on error (errno is set).
 */
VLC_API VLC_MALLOC char *vlc_obj_strdup(vlc_object_t *obj, const char *str);

/**
 * Manually frees module memory.
 *
 * This function manually frees a resource allocated with vlc_obj_malloc(),
 * vlc_obj_calloc() or vlc_obj_strdup() before the module instance is
 * terminated. This is seldom necessary.
 *
 * @param obj VLC object that the allocation was tied to
 * @param ptr pointer to the allocated resource
 */
VLC_API void vlc_obj_free(vlc_object_t *obj, void *ptr);

/** @} */
/** @} */
