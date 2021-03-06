#include <array>
#include <fstream>
#include <stdexcept>
#include <string_view>

#include <Euclid/Util/Assert.h>

#include "IOHelpers.h"

namespace Euclid
{

namespace _impl
{

template<int N, typename FT>
static void read_vertex_properties(std::ifstream& stream,
                                   std::vector<FT>* buffer)
{
    for (int i = 0; i < N; ++i) {
        FT value;
        stream >> value;
        buffer->push_back(value);
    }
}

} // namespace _impl

template<typename FT>
void read_obj(const std::string& filename,
              std::vector<FT>& positions,
              std::vector<FT>* texcoords,
              std::vector<FT>* normals)
{
    std::ifstream stream(filename);
    _impl::check_fstream(stream, filename);

    while (!stream.eof()) {
        std::string specifier;
        stream >> specifier;
        if (specifier == "v") {
            _impl::read_vertex_properties<3>(stream, &positions);
        }
        else if (specifier == "vt" && texcoords != nullptr) {
            _impl::read_vertex_properties<2>(stream, texcoords);
        }
        else if (specifier == "vn" && normals != nullptr) {
            _impl::read_vertex_properties<3>(stream, normals);
        }
        else {
            std::string dummy;
            std::getline(stream, dummy);
        }
    }
}

template<int N, typename FT, typename IT>
void read_obj(const std::string& filename,
              std::vector<FT>& positions,
              std::vector<IT>& pindices,
              std::vector<FT>* texcoords,
              std::vector<IT>* tindices,
              std::vector<FT>* normals,
              std::vector<IT>* nindices)
{
    std::ifstream stream(filename);
    _impl::check_fstream(stream, filename);

    while (!stream.eof()) {
        std::string specifier;
        stream >> specifier;
        if (specifier == "#") {
            std::string buffer;
            std::getline(stream, buffer);
        }
        else if (specifier == "v") {
            _impl::read_vertex_properties<3>(stream, &positions);
        }
        else if (specifier == "vt" && texcoords != nullptr) {
            _impl::read_vertex_properties<2>(stream, texcoords);
        }
        else if (specifier == "vn" && normals != nullptr) {
            _impl::read_vertex_properties<3>(stream, normals);
        }
        else if (specifier == "f") {
            std::getline(stream, specifier);
            auto faces = _impl::split(specifier, ' ', 1);
            if (faces.size() != N) {
                std::string err_str(
                    "Input file contains a face that is not a ");
                err_str.append(std::to_string(N)).append("-polygon");
                throw std::runtime_error(err_str);
            }
            for (const auto& face : faces) {
                auto idx = _impl::split(face, '/');
                if (idx.size() >= 1) {
                    pindices.push_back(std::stoi(std::string(idx[0])) - 1);
                }
                if (idx.size() >= 2 && tindices != nullptr) {
                    tindices->push_back(std::stoi(std::string(idx[1])) - 1);
                }
                if (idx.size() == 3 && nindices != nullptr) {
                    nindices->push_back(std::stoi(std::string(idx[2])) - 1);
                }
                if (idx.size() > 3) {
                    throw std::runtime_error("Bad obj file");
                }
            }
        }
        else {
            std::string dummy;
            std::getline(stream, dummy);
        }
    }
}

template<typename FT>
void write_obj(const std::string& filename,
               const std::vector<FT>& positions,
               const std::vector<FT>* texcoords,
               const std::vector<FT>* normals)
{
    std::ofstream stream(filename);
    _impl::check_fstream(stream, filename);

    if (positions.size() % 3 != 0) {
        throw std::runtime_error("Size of input is not valid.");
    }
    if (texcoords != nullptr && texcoords->size() % 2 != 0) {
        throw std::runtime_error("Size of input is not valid.");
    }
    if (normals != nullptr && normals->size() % 3 != 0) {
        throw std::runtime_error("Size of input is not valid.");
    }

    stream << "# Generated by Euclid(https://github.com/unclejimbo/Euclid)"
           << std::endl;
    for (size_t i = 0; i < positions.size(); i += 3) {
        stream << "v " << positions[i + 0] << " " << positions[i + 1] << " "
               << positions[i + 2] << std::endl;
    }
    if (texcoords != nullptr) {
        for (size_t i = 0; i < texcoords->size(); i += 2) {
            stream << "vt " << (*texcoords)[i + 0] << " " << (*texcoords)[i + 1]
                   << std::endl;
        }
    }
    if (normals != nullptr) {
        for (size_t i = 0; i < normals->size(); i += 3) {
            stream << "vn " << (*normals)[i + 0] << " " << (*normals)[i + 1]
                   << " " << (*normals)[i + 2] << std::endl;
        }
    }
}

template<int N, typename FT, typename IT>
void write_obj(const std::string& filename,
               const std::vector<FT>& positions,
               const std::vector<IT>& pindices,
               const std::vector<FT>* texcoords,
               const std::vector<IT>* tindices,
               const std::vector<FT>* normals,
               const std::vector<IT>* nindices)
{
    std::ofstream stream(filename);
    _impl::check_fstream(stream, filename);

    if (positions.size() % 3 != 0) {
        throw std::runtime_error("Size of input is not valid.");
    }
    if (texcoords != nullptr && texcoords->size() % 2 != 0) {
        throw std::runtime_error("Size of input is not valid.");
    }
    if (normals != nullptr && normals->size() % 3 != 0) {
        throw std::runtime_error("Size of input is not valid.");
    }
    if ((tindices != nullptr && tindices->size() != pindices.size()) ||
        (nindices != nullptr && nindices->size() != pindices.size())) {
        throw std::runtime_error(
            "The input indices are not compatible in size");
    }
    if (pindices.size() % N != 0) {
        std::string err_str("The input is not a valid ");
        err_str.append(std::to_string(N));
        err_str.append("-polygon");
        throw std::runtime_error(err_str);
    }

    stream << "# Generated by Euclid(https://github.com/unclejimbo/Euclid)"
           << std::endl;
    for (size_t i = 0; i < positions.size(); i += 3) {
        stream << "v " << positions[i + 0] << " " << positions[i + 1] << " "
               << positions[i + 2] << std::endl;
    }
    if (texcoords != nullptr) {
        for (size_t i = 0; i < texcoords->size(); i += 2) {
            stream << "vt " << (*texcoords)[i + 0] << " " << (*texcoords)[i + 1]
                   << std::endl;
        }
    }
    if (normals != nullptr) {
        for (size_t i = 0; i < normals->size(); i += 3) {
            stream << "vn " << (*normals)[i + 0] << " " << (*normals)[i + 1]
                   << " " << (*normals)[i + 2] << std::endl;
        }
    }
    for (size_t i = 0; i < pindices.size(); i += N) {
        stream << "f ";
        for (int j = 0; j < N; ++j) {
            stream << pindices[i + j] + 1;
            if (texcoords != nullptr && tindices != nullptr) {
                stream << "/" << (*tindices)[i + j] + 1;
                if (normals != nullptr && nindices != nullptr) {
                    stream << "/" << (*nindices)[i + j] + 1;
                }
            }
            else {
                if (normals != nullptr && nindices != nullptr) {
                    stream << "//" << (*nindices)[i + j] + 1;
                }
            }
            if (j < N - 1) { stream << " "; }
        }
        stream << std::endl;
    }
}

} // namespace Euclid
