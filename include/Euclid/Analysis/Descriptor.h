/** Shape descriptors.
 *
 *  This package contain functions to compute shape descriptors.
 *  @defgroup PkgDescriptor Descriptor
 *  @ingroup PkgAnalysis
 */
#pragma once

#include <vector>
#include <CGAL/boost/graph/properties.h>

namespace Euclid
{
/** @{*/

/** The spin image descriptor.
 *
 *  Spin image is an image-based local shape descriptor. For every point
 *  on a mesh, an image is generated by projecting points onto the image
 *  plane within a local support.
 *
 *  @param mesh Input mesh.
 *  @param vnmap The vertex normal property map.
 *  @param v The vertex descriptor.
 *  @param spin_img The output spin image for v.
 *  @param bin_size Multiples of average edge length.
 *  @param image_width Number of rows and colums for the image.
 *  @param support_angle Maximum support angle in degrees.
 *
 *  #### Reference
 *  Johnson A E, Hebert M.
 *  Using spin images for efficient object recognition in cluttered 3D
 * 	scenes[J].
 *  IEEE Transactions on pattern analysis and machine intelligence,
 *  1999, 21(5): 433-449.
 */
template<typename Mesh, typename VertexNormalMap, typename T>
void spin_image(const Mesh& mesh,
                const VertexNormalMap& vnmap,
                const typename boost::graph_traits<Mesh>::vertex_descriptor& v,
                std::vector<T>& spin_img,
                float bin_size = 1.0f,
                unsigned image_width = 15,
                float support_angle = 60.0f);

/** @}*/
} // namespace Euclid

#include "src/SpinImage.cpp"
