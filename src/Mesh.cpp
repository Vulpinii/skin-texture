#include "Mesh.hpp"

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// constructors
Mesh::Mesh()= default;

Mesh::Mesh(const char * filename)
{
    bounding_box = BOX();

    load_OFF_file(filename, indexed_vertices, indexed_normals, indices, triangles,
                  bounding_box.xpos, bounding_box.ypos, bounding_box.zpos);
    std::cout << "**********\nBounding box :" << std::endl;
    std::cout << "(xmin, xmax) = (" << bounding_box.xpos.x << ", " << bounding_box.xpos.y << ")" << std::endl;
    std::cout << "(ymin, ymax) = (" << bounding_box.ypos.x << ", " << bounding_box.ypos.y << ")" << std::endl;
    std::cout << "(zmin, zmax) = (" << bounding_box.zpos.x << ", " << bounding_box.zpos.y << ")" << std::endl;
    std::cout << "**********" << std::endl;
    indexed_uvs.resize(indexed_vertices.size(), glm::vec2(1.)); //List vide de UV

    compute_smooth_vertex_normals(0);
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// destructor
Mesh::~Mesh() = default;

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// normal computation
void Mesh::compute_smooth_vertex_normals(int weight_type)
{
    compute_smooth_vertex_normals(indexed_vertices, triangles, weight_type, indexed_normals);
}


void Mesh::compute_triangle_normals (const std::vector<glm::vec3> & vertices,
                                     const std::vector<std::vector<unsigned short> > & triangles,
                                     std::vector<glm::vec3> & triangle_normals)
{
    for(auto triangle : triangles)
    {
        glm::vec3 p0 = vertices.at(triangle.at(0));
        glm::vec3 p1 = vertices.at(triangle.at(1));
        glm::vec3 p2 = vertices.at(triangle.at(2));

        glm::vec3 n = glm::normalize(glm::cross(p1-p0, p2-p0));

        triangle_normals.push_back(n);
    }
}

void Mesh::compute_smooth_vertex_normals (const std::vector<glm::vec3> & vertices,
                                          const std::vector<std::vector<unsigned short> > & triangles,
                                          unsigned int weight_type, //0 uniforme, 1 area of triangles, 2 angle of triangle
                                          std::vector<glm::vec3> & vertex_normals){

    vertex_normals.clear();
    vertex_normals.resize(vertices.size(), glm::vec3(0.0));

    std::vector<glm::vec3> triangle_normals;
    std::vector<glm::vec3> triangle_angles;
    std::vector<float> triangle_surface, point_aire_triangles, point_angles_triangles;

    compute_triangle_normals(vertices, triangles, triangle_normals);
    triangle_angles.resize(triangles.size(), glm::vec3(0.0));
    triangle_surface.resize(triangles.size(),0.0);
    point_aire_triangles.resize(vertices.size(),0.0f);
    point_angles_triangles.resize(vertices.size(), 0.0f);

    for(unsigned int i = 0; i < triangles.size(); ++i)
    {
        glm::vec3 p0 = vertices.at(triangles.at(i).at(0));
        glm::vec3 p1 = vertices.at(triangles.at(i).at(1));
        glm::vec3 p2 = vertices.at(triangles.at(i).at(2));

        switch(weight_type){
            case 0 :
                // we add normal of the current triangle to each vertex
                vertex_normals.at(triangles.at(i).at(0)) += triangle_normals.at(i);
                vertex_normals.at(triangles.at(i).at(1)) += triangle_normals.at(i);
                vertex_normals.at(triangles.at(i).at(2)) += triangle_normals.at(i);
                break;

            case 1:
                // we add area of a triangle to each vertices
                triangle_surface.at(i) = glm::dot(p1-p0, p2-p0)/2.0f;
                point_aire_triangles.at(triangles.at(i).at(0)) += triangle_surface.at(i);
                point_aire_triangles.at(triangles.at(i).at(1)) += triangle_surface.at(i);
                point_aire_triangles.at(triangles.at(i).at(2)) += triangle_surface.at(i);
                break;

            case 2:
                // we add near triangle's area with current triangle of a vertex
                triangle_angles.at(i).x = acos(glm::radians(glm::dot(p1-p0, p2-p0)/
                                                            (glm::length(p1-p0) * glm::length(p2-p0))));
                triangle_angles.at(i).y = acos(glm::radians(glm::dot(p2-p1, p0-p1)/
                                                            (glm::length(p0-p1) * glm::length(p2-p1))));
                triangle_angles.at(i).z = acos(glm::radians(glm::dot(p0-p2, p1-p2)/
                                                            (glm::length(p0-p2) * glm::length(p1-p2))));

                point_angles_triangles.at(triangles.at(i).at(0)) += triangle_angles.at(i).x;
                point_angles_triangles.at(triangles.at(i).at(1)) += triangle_angles.at(i).y;
                point_angles_triangles.at(triangles.at(i).at(2)) += triangle_angles.at(i).z;
                break;
        }
    }

    switch(weight_type){
        case 1 :
            // we divide the weight of the normal for each triangle with the area
            for(int i = 0 ; i < triangles.size() ; i++){
                vertex_normals.at(triangles.at(i).at(0)) += triangle_normals.at(i)*
                                                            (triangle_surface.at(i)/point_aire_triangles.at(triangles.at(i).at(0)));
                vertex_normals.at(triangles.at(i).at(1)) += triangle_normals.at(i)*
                                                            (triangle_surface.at(i)/point_aire_triangles.at(triangles.at(i).at(1)));
                vertex_normals.at(triangles.at(i).at(2)) += triangle_normals.at(i)*
                                                            (triangle_surface.at(i)/point_aire_triangles.at(triangles.at(i).at(2)));
            }
            break;

        case 2 :
            // we devide the weight of the normal with each vertex depending of the angle of
            // near triangles normalize with max angle
            for(int i = 0 ; i < triangles.size() ; i++){
                vertex_normals.at(triangles.at(i).at(0)) += triangle_normals.at(i)*
                                                            (triangle_angles.at(i).x/point_angles_triangles.at(triangles.at(i).at(0)));
                vertex_normals.at(triangles.at(i).at(1)) += triangle_normals.at(i)*
                                                            (triangle_angles.at(i).y/point_angles_triangles.at(triangles.at(i).at(1)));
                vertex_normals.at(triangles.at(i).at(2)) += triangle_normals.at(i)*
                                                            (triangle_angles.at(i).z/point_angles_triangles.at(triangles.at(i).at(2)));
            }
            break;
    }

    // we nomalize normals
    for(unsigned int i = 0 ; i < vertex_normals.size(); ++i)
    {
        vertex_normals.at(i) = glm::normalize(vertex_normals.at(i));
    }
}

void Mesh::collect_one_ring (const std::vector<glm::vec3> & vertices,
                             const std::vector<std::vector<unsigned short> > & triangles,
                             std::vector<std::vector<unsigned short> > & one_ring)
{
    one_ring.resize(vertices.size());

    for (unsigned int i = 0 ; i < triangles.size() ; ++i)
    {
        if (std::find(one_ring.at(triangles.at(i).at(0)).begin(),
                      one_ring.at(triangles.at(i).at(0)).end(),
                      triangles.at(i).at(1)) == one_ring.at(triangles.at(i).at(0)).end())
            one_ring.at(triangles.at(i).at(0)).push_back(triangles.at(i).at(1));

        if (std::find(one_ring.at(triangles.at(i).at(0)).begin(),
                      one_ring.at(triangles.at(i).at(0)).end(),
                      triangles.at(i).at(2)) == one_ring.at(triangles.at(i).at(0)).end())
            one_ring.at(triangles.at(i).at(0)).push_back(triangles.at(i).at(2));

        if (std::find(one_ring.at(triangles.at(i).at(1)).begin(),
                      one_ring.at(triangles.at(i).at(1)).end(),
                      triangles.at(i).at(0)) == one_ring.at(triangles.at(i).at(1)).end())
            one_ring.at(triangles.at(i).at(1)).push_back(triangles.at(i).at(0));

        if (std::find(one_ring.at(triangles.at(i).at(1)).begin(),
                      one_ring.at(triangles.at(i).at(1)).end(),
                      triangles.at(i).at(2)) == one_ring.at(triangles.at(i).at(1)).end())
            one_ring.at(triangles.at(i).at(1)).push_back(triangles.at(i).at(2));

        if (std::find(one_ring.at(triangles.at(i).at(2)).begin(),
                      one_ring.at(triangles.at(i).at(2)).end(),
                      triangles.at(i).at(0)) == one_ring.at(triangles.at(i).at(2)).end())
            one_ring.at(triangles.at(i).at(2)).push_back(triangles.at(i).at(0));

        if (std::find(one_ring.at(triangles.at(i).at(2)).begin(),
                      one_ring.at(triangles.at(i).at(2)).end(),
                      triangles.at(i).at(1)) == one_ring.at(triangles.at(i).at(2)).end())
            one_ring.at(triangles.at(i).at(2)).push_back(triangles.at(i).at(1));
    }
}


// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// load off file


bool Mesh::load_OFF_file(const std::string & filename, std::vector< glm::vec3 > & vertices,
                         std::vector< glm::vec3 > & normals, std::vector< unsigned short > & indices,
                         std::vector< std::vector<unsigned short > > & triangles,
                         glm::vec2 & xpos, glm::vec2 & ypos, glm::vec2 & zpos)
{
    std::ifstream myfile; myfile.open(filename.c_str());
    if (!myfile.is_open())
    {
        std::cout << "Failure to open " <<filename << " file"<< std::endl;
        return false;
    }

    std::string isOFFfile; myfile >> isOFFfile;
    if(isOFFfile != "OFF")
    {
        std::cerr << "File " << filename << " isn't an OFF format file" << std::endl;
        myfile.close();
        return false;
    }

    int numberOfVertices , numberOfFaces , numberOfEdges;
    myfile >> numberOfVertices >> numberOfFaces >> numberOfEdges;

    vertices.resize(numberOfVertices);
    normals.resize(numberOfVertices);

    std::vector< std::vector<glm::vec3> > normalsTmp;
    normalsTmp.resize(numberOfVertices);


    for( int v = 0 ; v < numberOfVertices ; ++v )
    {
        glm::vec3 vertex;
        myfile >> vertex.x >> vertex.y >> vertex.z;
        vertices[v] = vertex;

        if(v == 0)
        {
            xpos.x = vertex.x; xpos.y = vertex.x;
            ypos.x = vertex.y; ypos.y = vertex.y;
            zpos.x = vertex.z; zpos.y = vertex.z;
        }
        else
        {
            if(xpos.x > vertex.x) xpos.x = vertex.x;
            if(xpos.y < vertex.x) xpos.y = vertex.x;

            if(ypos.x > vertex.y) ypos.x = vertex.y;
            if(ypos.y < vertex.y) ypos.y = vertex.y;

            if(zpos.x > vertex.z) zpos.x = vertex.z;
            if(zpos.y < vertex.z) zpos.y = vertex.z;
        }
    }


    for( int f = 0 ; f < numberOfFaces ; ++f )
    {
        int numberOfVerticesOnFace;
        myfile >> numberOfVerticesOnFace;
        if( numberOfVerticesOnFace == 3 )
        {
            unsigned short v1 , v2 , v3;
            std::vector< unsigned short > v;
            myfile >> v1 >> v2 >> v3;

            v.push_back(v1);
            v.push_back(v2);
            v.push_back(v3);

            triangles.push_back(v);

            indices.push_back(v1);
            indices.push_back(v2);
            indices.push_back(v3);

            glm::vec3 normal;
            glm::vec3 tmp1 = vertices[v2] - vertices[v1];
            glm::vec3 tmp2 = vertices[v3] - vertices[v1];
            normal = glm::normalize(glm::cross(tmp1,tmp2));
            normalsTmp[v1].push_back(normal);
            normalsTmp[v2].push_back(normal);
            normalsTmp[v3].push_back(normal);

        }
        else
        {
            std::cerr << "Number of vertices on face must be 3" << std::endl;
            myfile.close();
            return false;
        }
    }

    for( int v = 0 ; v < numberOfVertices ; ++v )
    {
        float sizeTmp = normalsTmp[v].size();
        glm::vec3 tmp;
        for(auto n : normalsTmp[v])
        {
            tmp += n;
        }
        tmp = tmp / sizeTmp;
        normals[v] = tmp;
    }

    myfile.close();
    return true;
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// simplify vertices / normals of the mesh

glm::vec4 Mesh::equation_plane(float x1, float y1, float z1,
                               float x2, float y2, float z2,
                               float x3, float y3, float z3)
{
    float a1 = x2 - x1;
    float b1 = y2 - y1;
    float c1 = z2 - z1;

    float a2 = x3 - x1;
    float b2 = y3 - y1;
    float c2 = z3 - z1;

    float a = b1 * c2 - b2 * c1;
    float b = a2 * c1 - a1 * c2;
    float c = a1 * b2 - b1 * a2;

    float d = (- a * x1 - b * y1 - c * z1);
    return glm::vec4(a,b,c,d);
}