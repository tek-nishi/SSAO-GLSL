#pragma once

//
// 頂点カラーで陰影をつける
//

#include <glm/gtc/random.hpp>
#include "triMesh.hpp"


// メッシュごとに頂点カラーで陰影を計算
void makeVertexShadow(TriMesh& mesh) {
  auto& vertex_colors = mesh.getColorsRGBA();
  vertex_colors.clear();


  const auto& positions = mesh.getPositions();
  const auto& normals   = mesh.getNormals();
  const auto& indices   = mesh.getIndices();

  float light;
  for (u_int i = 0; i < positions.size(); ++i) {
    // アンビエントオクルージョン的な計算
    const auto& p = positions[i];
    const auto& n = normals[i];
    for (u_int j = 0; j < 100; ++j) {
      ci::vec3 v = glm::sphericalRand(1.0f);

      if (glm::dot(v, n) < 0.0f) continue;
      
      ci::Ray ray(p, v);
      for (u_int k = 0; k < (indices.size() / 3); ++k) {
        float d;
        if (ray.calcTriangleIntersection(positions[indices[k * 3 + 0]],
                                         positions[indices[k * 3 + 1]],
                                         positions[indices[k * 3 + 2]],
                                         &d)) {
          continue;
        }

        // 交差が無いなら明るさを加算
        light += 1.0f;
      }
    }
    light /= 100.0f * (indices.size() / 3);
    ci::ColorA color(light, light, light, 1.0f);
    mesh.appendColorRGBA(color);
  }
}
