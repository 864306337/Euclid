#include <cmath>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/functional/hash.hpp>
#include <boost/math/constants/constants.hpp>
#include <CGAL/boost/graph/helpers.h>
#include <Euclid/Math/Vector.h>
#include <Euclid/Util/Assert.h>

namespace Euclid
{

template<typename Mesh, typename Vector_3>
Vector_3 vertex_normal(
    typename boost::graph_traits<const Mesh>::vertex_descriptor v,
    const Mesh& mesh,
    const VertexNormal& weight)
{
    auto vpmap = get(boost::vertex_point, mesh);
    Vector_3 normal(0.0, 0.0, 0.0);
    for (auto he : halfedges_around_source(v, mesh)) {
        if (!CGAL::is_border(he, mesh)) {
            auto f = face(he, mesh);
            auto fn = face_normal(f, mesh);

            if (weight == VertexNormal::uniform) { normal += fn; }
            else if (weight == VertexNormal::face_area) {
                auto area = face_area(f, mesh);
                normal += area * fn;
            }
            else { // incident_angle
                auto he_next = next(he, mesh);
                auto t = target(he, mesh);
                auto s1 = source(he, mesh);
                auto s2 = target(he_next, mesh);
                auto pt = get(vpmap, t);
                auto ps1 = get(vpmap, s1);
                auto ps2 = get(vpmap, s2);
                auto vec1 = normalized(ps1 - pt);
                auto vec2 = normalized(ps2 - pt);
                auto angle = std::acos(vec1 * vec2);
                normal += angle * fn;
            }
        }
    }
    return Euclid::normalized(normal);
}

template<typename Mesh, typename Vector_3>
Vector_3 vertex_normal(
    typename boost::graph_traits<const Mesh>::vertex_descriptor v,
    const Mesh& mesh,
    const std::vector<Vector_3>& face_normals,
    const VertexNormal& weight)
{
    auto vpmap = get(boost::vertex_point, mesh);
    auto fimap = get(boost::face_index, mesh);
    Vector_3 normal(0.0, 0.0, 0.0);
    for (auto he : halfedges_around_source(v, mesh)) {
        if (!CGAL::is_border(he, mesh)) {
            auto f = face(he, mesh);
            auto fi = get(fimap, f);
            auto fn = face_normals[fi];

            if (weight == VertexNormal::uniform) { normal += fn; }
            else if (weight == VertexNormal::face_area) {
                auto area = face_area(f, mesh);
                normal += area * fn;
            }
            else { // incident_angle
                auto he_next = next(he, mesh);
                auto t = target(he, mesh);
                auto s1 = source(he, mesh);
                auto s2 = target(he_next, mesh);
                auto pt = get(vpmap, t);
                auto ps1 = get(vpmap, s1);
                auto ps2 = get(vpmap, s2);
                auto vec1 = normalized(ps1 - pt);
                auto vec2 = normalized(ps2 - pt);
                auto angle = std::acos(vec1 * vec2);
                normal += angle * fn;
            }
        }
    }
    return Euclid::normalized(normal);
}

template<typename Mesh, typename Vector_3>
std::vector<Vector_3> vertex_normals(const Mesh& mesh,
                                     const std::vector<Vector_3>& face_normals,
                                     const VertexNormal& weight)
{
    std::vector<Vector_3> vnormals;
    vnormals.reserve(num_vertices(mesh));
    for (auto v : vertices(mesh)) {
        vnormals.push_back(vertex_normal(v, mesh, face_normals, weight));
    }
    return vnormals;
}

template<typename Mesh, typename T>
T vertex_area(typename boost::graph_traits<const Mesh>::vertex_descriptor v,
              const Mesh& mesh,
              const VertexArea& method)
{
    auto vpmap = get(boost::vertex_point, mesh);
    auto va = T(0);
    if (method == VertexArea::barycentric) {
        const auto one_third = boost::math::constants::third<T>();
        for (auto he : halfedges_around_target(v, mesh)) {
            if (!CGAL::is_border(he, mesh)) {
                auto p1 = get(vpmap, source(he, mesh));
                auto p2 = get(vpmap, target(he, mesh));
                auto p3 = get(vpmap, target(next(he, mesh), mesh));
                va += area(p1, p2, p3);
            }
        }
        va *= one_third;
    }
    else if (method == VertexArea::voronoi) {
        for (auto he : halfedges_around_target(v, mesh)) {
            if (!CGAL::is_border(he, mesh)) {
                auto p1 = get(vpmap, source(he, mesh));
                auto p2 = get(vpmap, target(he, mesh));
                auto p3 = get(vpmap, target(next(he, mesh), mesh));
                auto mid1 = CGAL::midpoint(p2, p1);
                auto mid2 = CGAL::midpoint(p2, p3);
                auto center = CGAL::circumcenter(p1, p2, p3);
                if (CGAL::angle(p2, p1, p3) == CGAL::OBTUSE) {
                    va += area(mid1, p2, center) - area(mid2, center, p2);
                }
                else if (CGAL::angle(p2, p3, p1) == CGAL::OBTUSE) {
                    va += area(mid2, center, p2) - area(mid1, p2, center);
                }
                else {
                    va += area(mid1, p2, center) + area(mid2, center, p2);
                }
            }
        }
    }
    else { // method == VertexArea::mixed_voronoi
        for (auto he : halfedges_around_target(v, mesh)) {
            if (!CGAL::is_border(he, mesh)) {
                auto p1 = get(vpmap, source(he, mesh));
                auto p2 = get(vpmap, target(he, mesh));
                auto p3 = get(vpmap, target(next(he, mesh), mesh));
                if (CGAL::angle(p1, p2, p3) == CGAL::OBTUSE) {
                    va += area(p1, p2, p3) * 0.5;
                }
                else if (CGAL::angle(p3, p1, p2) == CGAL::OBTUSE ||
                         CGAL::angle(p1, p3, p2) == CGAL::OBTUSE) {
                    va += area(p1, p2, p3) * 0.25;
                }
                else { // triangle is acute or right
                    auto mid1 = CGAL::midpoint(p2, p1);
                    auto mid2 = CGAL::midpoint(p2, p3);
                    auto center = CGAL::circumcenter(p1, p2, p3);
                    va += area(mid1, p2, center) + area(mid2, center, p2);
                }
            }
        }
    }
    return va;
}

template<typename Mesh, typename T>
std::vector<T> vertex_areas(const Mesh& mesh, const VertexArea& method)
{
    std::vector<T> vareas;
    vareas.reserve(num_vertices(mesh));
    for (auto v : vertices(mesh)) {
        vareas.push_back(vertex_area(v, mesh, method));
    }
    return vareas;
}

template<typename Mesh, typename T>
T edge_length(typename boost::graph_traits<const Mesh>::halfedge_descriptor he,
              const Mesh& mesh)
{
    auto vpmap = get(boost::vertex_point, mesh);
    auto p1 = get(vpmap, source(he, mesh));
    auto p2 = get(vpmap, target(he, mesh));
    return length(p2 - p1);
}

template<typename Mesh, typename T>
T edge_length(typename boost::graph_traits<const Mesh>::edge_descriptor e,
              const Mesh& mesh)
{
    auto he = halfedge(e, mesh);
    return edge_length(he, mesh);
}

template<typename Mesh, typename T>
std::vector<T> edge_lengths(const Mesh& mesh)
{
    std::vector<T> elens;
    elens.reserve(num_edges(mesh));
    for (auto he : edges(mesh)) {
        elens.push_back(edge_length(he, mesh));
    }
    return elens;
}

template<typename Mesh, typename T>
T squared_edge_length(
    typename boost::graph_traits<const Mesh>::halfedge_descriptor he,
    const Mesh& mesh)
{
    auto vpmap = get(boost::vertex_point, mesh);
    auto p1 = get(vpmap, source(he, mesh));
    auto p2 = get(vpmap, target(he, mesh));
    return (p1 - p2).squared_length();
}

template<typename Mesh, typename T>
T squared_edge_length(
    typename boost::graph_traits<const Mesh>::edge_descriptor e,
    const Mesh& mesh)
{
    auto he = halfedge(e, mesh);
    return squared_edge_length(he, mesh);
}

template<typename Mesh, typename T>
std::vector<T> squared_edge_lengths(const Mesh& mesh)
{
    std::vector<T> elens;
    elens.reserve(num_edges(mesh));
    for (auto he : edges(mesh)) {
        elens.push_back(squared_edge_length(he, mesh));
    }
    return elens;
}

template<typename Mesh, typename Vector_3>
Vector_3 face_normal(
    typename boost::graph_traits<const Mesh>::face_descriptor f,
    const Mesh& mesh)
{
    auto vpmap = get(boost::vertex_point, mesh);
    auto he = halfedge(f, mesh);
    auto p1 = get(vpmap, source(he, mesh));
    auto p2 = get(vpmap, target(he, mesh));
    auto p3 = get(vpmap, target(next(he, mesh), mesh));

    Vector_3 result;
    if (CGAL::collinear(p1, p2, p3)) {
        EWARNING("Degenerate face, normal is set to zero vector");
        result = Vector_3(0.0, 0.0, 0.0);
    }
    else {
        result = normalized(CGAL::normal(p1, p2, p3));
    }
    return result;
}

template<typename Mesh, typename Vector_3>
std::vector<Vector_3> face_normals(const Mesh& mesh)
{
    std::vector<Vector_3> fnormals;
    fnormals.reserve(num_faces(mesh));
    for (auto f : faces(mesh)) {
        fnormals.push_back(face_normal(f, mesh));
    }
    return fnormals;
}

template<typename Mesh, typename T>
T face_area(typename boost::graph_traits<const Mesh>::face_descriptor f,
            const Mesh& mesh)
{
    auto vpmap = get(boost::vertex_point, mesh);
    auto he = halfedge(f, mesh);
    auto p1 = get(vpmap, source(he, mesh));
    auto p2 = get(vpmap, target(he, mesh));
    auto p3 = get(vpmap, target(next(he, mesh), mesh));
    return area(p1, p2, p3);
}

template<typename Mesh, typename T>
std::vector<T> face_areas(const Mesh& mesh)
{
    std::vector<T> fareas;
    fareas.reserve(num_faces(mesh));
    for (auto f : faces(mesh)) {
        fareas.push_back(face_area(f, mesh));
    }
    return fareas;
}

template<typename Mesh, typename Point_3>
Point_3 barycenter(typename boost::graph_traits<const Mesh>::face_descriptor f,
                   const Mesh& mesh)
{
    auto vpmap = get(boost::vertex_point, mesh);
    auto [vbeg, vend] = vertices_around_face(halfedge(f, mesh), mesh);
    auto p0 = get(vpmap, *vbeg++);
    auto p1 = get(vpmap, *vbeg++);
    auto p2 = get(vpmap, *vbeg);
    return CGAL::centroid(p0, p1, p2);
}

template<typename Mesh, typename Point_3>
std::vector<Point_3> barycenters(const Mesh& mesh)
{
    std::vector<Point_3> centroids;
    centroids.reserve(num_faces(mesh));
    for (auto f : faces(mesh)) {
        centroids.push_back(barycenter(f, mesh));
    }
    return centroids;
}

template<typename Mesh, typename T>
T gaussian_curvature(
    typename boost::graph_traits<const Mesh>::vertex_descriptor v,
    const Mesh& mesh)
{
    auto vpmap = get(boost::vertex_point, mesh);

    T angle_defect = boost::math::constants::two_pi<T>();
    for (auto he : halfedges_around_target(v, mesh)) {
        if (!CGAL::is_border(he, mesh)) {
            auto vp = source(he, mesh);
            auto vq = target(next(he, mesh), mesh);
            angle_defect -= std::acos(
                cosine(get(vpmap, vp), get(vpmap, v), get(vpmap, vq)));
        }
    }
    return angle_defect / vertex_area(v, mesh);
}

template<typename Mesh, typename T>
std::vector<T> gaussian_curvatures(const Mesh& mesh)
{
    std::vector<T> curvatures;
    curvatures.reserve(num_vertices(mesh));
    for (auto v : vertices(mesh)) {
        curvatures.push_back(gaussian_curvature(v, mesh));
    }
    return curvatures;
}

template<typename Mesh, typename T>
std::tuple<Eigen::SparseMatrix<T>, Eigen::SparseMatrix<T>> adjacency_matrix(
    const Mesh& mesh)
{
    using Triplet = Eigen::Triplet<T>;
    auto vimap = get(boost::vertex_index, mesh);
    const auto nv = num_vertices(mesh);

    std::vector<Triplet> adj, degree;
    for (auto vi : vertices(mesh)) {
        int i = get(vimap, vi);
        int d = 0;
        for (auto he : halfedges_around_target(vi, mesh)) {
            auto vj = source(he, mesh);
            int j = get(vimap, vj);
            adj.emplace_back(i, j, 1);
            ++d;
        }
        degree.emplace_back(i, i, d);
    }

    Eigen::SparseMatrix<T> adj_mat(nv, nv), degree_mat(nv, nv);
    adj_mat.setFromTriplets(adj.begin(), adj.end());
    adj_mat.makeCompressed();
    degree_mat.setFromTriplets(degree.begin(), degree.end());
    degree_mat.makeCompressed();
    return std::make_tuple(adj_mat, degree_mat);
}

template<typename Mesh, typename T>
Eigen::SparseMatrix<T> cotangent_matrix(const Mesh& mesh)
{
    using Triplet = Eigen::Triplet<T>;
    auto vpmap = get(boost::vertex_point, mesh);
    auto vimap = get(boost::vertex_index, mesh);
    const auto nv = num_vertices(mesh);

    auto hash_fcn = [](const Triplet& t) {
        size_t seed = 0;
        boost::hash_combine(seed, t.col());
        boost::hash_combine(seed, t.row());
        return seed;
    };
    auto eq_fcn = [](const Triplet& t1, const Triplet& t2) {
        return (t1.col() == t2.col()) && (t1.row() == t2.row());
    };
    std::unordered_set<Triplet, decltype(hash_fcn), decltype(eq_fcn)> values(
        nv, hash_fcn, eq_fcn);

    for (auto vi : vertices(mesh)) {
        int i = get(vimap, vi);
        T row_sum = 0.0;
        for (auto he : halfedges_around_target(vi, mesh)) {
            auto vj = source(he, mesh);
            int j = get(vimap, vj);
            auto existing = values.find(Triplet(j, i, 0.0));
            if (existing != values.end()) {
                values.emplace(i, j, existing->value());
                row_sum -= existing->value();
            }
            else {
                auto va = target(next(he, mesh), mesh);
                auto vb = target(next(opposite(he, mesh), mesh), mesh);
                auto cota =
                    cotangent(get(vpmap, vi), get(vpmap, va), get(vpmap, vj));
                auto cotb =
                    cotangent(get(vpmap, vi), get(vpmap, vb), get(vpmap, vj));
                auto value = static_cast<T>((cota + cotb) * 0.5);
                values.emplace(i, j, -value);
                row_sum += value;
            }
        }
        values.emplace(i, i, row_sum);
    }

    Eigen::SparseMatrix<T> mat(nv, nv);
    mat.setFromTriplets(values.begin(), values.end());
    mat.makeCompressed();
    return mat;
}

template<typename Mesh, typename T>
Eigen::SparseMatrix<T> mass_matrix(const Mesh& mesh, const VertexArea& method)
{
    const auto nv = num_vertices(mesh);
    Eigen::SparseMatrix<T> mass(nv, nv);
    std::vector<Eigen::Triplet<T>> values;

    int i = 0;
    for (auto v : vertices(mesh)) {
        auto area = vertex_area(v, mesh, method);
        values.emplace_back(i, i, area);
        ++i;
    }

    mass.setFromTriplets(values.begin(), values.end());
    mass.makeCompressed();
    return mass;
}

} // namespace Euclid
