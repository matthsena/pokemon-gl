#include "window.hpp"

#include <unordered_map>

// Explicit specialization of std::hash for Vertex
template <> struct std::hash<Vertex> {
  size_t operator()(Vertex const &vertex) const noexcept {
    auto const h1{std::hash<glm::vec3>()(vertex.position)};
    return h1;
  }
};

void Window::onEvent(SDL_Event const &event) {
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_SPACE) {
      launchPokeball();
    }

    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
      m_dollySpeed = 1.0f;
    if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
      m_dollySpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_panSpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_panSpeed = 1.0f;
    if (event.key.keysym.sym == SDLK_q)
      m_truckSpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_e)
      m_truckSpeed = 1.0f;
  }
  if (event.type == SDL_KEYUP) {
    if ((event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) &&
        m_dollySpeed > 0)
      m_dollySpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) &&
        m_dollySpeed < 0)
      m_dollySpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) &&
        m_panSpeed < 0)
      m_panSpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_RIGHT ||
         event.key.keysym.sym == SDLK_d) &&
        m_panSpeed > 0)
      m_panSpeed = 0.0f;
    if (event.key.keysym.sym == SDLK_q && m_truckSpeed < 0)
      m_truckSpeed = 0.0f;
    if (event.key.keysym.sym == SDLK_e && m_truckSpeed > 0)
      m_truckSpeed = 0.0f;
  }
}

void Window::onCreate() {
  auto const &assetsPath{abcg::Application::getAssetsPath()};

  abcg::glClearColor(0, 0, 0, 1);

  // Enable depth buffering
  abcg::glEnable(GL_DEPTH_TEST);

  // Create program
  m_program =
      abcg::createOpenGLProgram({{.source = assetsPath + "lookat.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "lookat.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

  m_ground.create(m_program);

  // Get location of uniform variables
  m_viewMatrixLocation = abcg::glGetUniformLocation(m_program, "viewMatrix");
  m_projMatrixLocation = abcg::glGetUniformLocation(m_program, "projMatrix");
  m_modelMatrixLocation = abcg::glGetUniformLocation(m_program, "modelMatrix");
  m_colorLocation = abcg::glGetUniformLocation(m_program, "color");

  // Load model
  auto const [vertices_pokemon, indices_pokemon] =
      loadModelFromFile(assetsPath + "charmander.obj");
  m_vertices = vertices_pokemon;
  m_indices = indices_pokemon;

  // Generate VBO
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER,
                     sizeof(m_vertices.at(0)) * m_vertices.size(),
                     m_vertices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(m_indices.at(0)) * m_indices.size(),
                     m_indices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Create VAO
  abcg::glGenVertexArrays(1, &m_VAO);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_VAO);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  auto const positionAttribute{
      abcg::glGetAttribLocation(m_program, "inPosition")};
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex), nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);

  // build pokeball
  auto const [vertices_pokeball, indices_pokeball] =
      loadModelFromFile(assetsPath + "pokeball.obj");
  m_vertices_pokeball = vertices_pokeball;
  m_indices_pokeball = indices_pokeball;

  // Generate VBO
  abcg::glGenBuffers(1, &m_VBO_pokeball);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO_pokeball);
  abcg::glBufferData(GL_ARRAY_BUFFER,
                     sizeof(m_vertices_pokeball.at(0)) *
                         m_vertices_pokeball.size(),
                     m_vertices_pokeball.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  abcg::glGenBuffers(1, &m_EBO_pokeball);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_pokeball);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(m_indices_pokeball.at(0)) *
                         m_indices_pokeball.size(),
                     m_indices_pokeball.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Create VAO
  abcg::glGenVertexArrays(1, &m_VAO_pokeball);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_VAO_pokeball);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO_pokeball);
  auto const positionAttribute_pokeball{
      abcg::glGetAttribLocation(m_program, "inPosition")};
  abcg::glEnableVertexAttribArray(positionAttribute_pokeball);
  abcg::glVertexAttribPointer(positionAttribute_pokeball, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex), nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_pokeball);

  abcg::glBindVertexArray(0);

  // Definindo posição inicial dos pokemons
  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  std::uniform_real_distribution<float> rd_poke_position(-1.0f, 1.0f);

  m_pokemonPosition[0] = glm::vec3(rd_poke_position(m_randomEngine), 0,
                                   rd_poke_position(m_randomEngine));

  m_pokemonPosition[1] = glm::vec3(rd_poke_position(m_randomEngine), 0,
                                   rd_poke_position(m_randomEngine));

  m_pokemonPosition[2] = glm::vec3(rd_poke_position(m_randomEngine), 0,
                                   rd_poke_position(m_randomEngine));
}

// https://stackoverflow.com/questions/321068/returning-multiple-values-from-a-c-function
std::tuple<std::vector<Vertex>, std::vector<GLuint>>
Window::loadModelFromFile(std::string_view path) {
  tinyobj::ObjReader reader;
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;

  if (!reader.ParseFromFile(path.data())) {
    if (!reader.Error().empty()) {
      throw abcg::RuntimeError(
          fmt::format("Failed to load model {} ({})", path, reader.Error()));
    }
    throw abcg::RuntimeError(fmt::format("Failed to load model {}", path));
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  auto const &attributes{reader.GetAttrib()};
  auto const &shapes{reader.GetShapes()};

  vertices.clear();
  indices.clear();

  // A key:value map with key=Vertex and value=index
  std::unordered_map<Vertex, GLuint> hash{};

  // Loop over shapes
  for (auto const &shape : shapes) {
    // Loop over indices
    for (auto const offset : iter::range(shape.mesh.indices.size())) {
      // Access to vertex
      auto const index{shape.mesh.indices.at(offset)};

      // Vertex position
      auto const startIndex{3 * index.vertex_index};
      auto const vx{attributes.vertices.at(startIndex + 0)};
      auto const vy{attributes.vertices.at(startIndex + 1)};
      auto const vz{attributes.vertices.at(startIndex + 2)};

      Vertex const vertex{.position = {vx, vy, vz}};

      // If map doesn't contain this vertex
      if (!hash.contains(vertex)) {
        // Add this index (size of m_vertices)
        hash[vertex] = vertices.size();
        // Add this vertex
        vertices.push_back(vertex);
      }

      indices.push_back(hash[vertex]);
    }
  }

  return std::make_tuple(vertices, indices);
}

void Window::onPaint() {
  // Clear color buffer and depth buffer
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  abcg::glUseProgram(m_program);

  // Set uniform variables for viewMatrix and projMatrix
  // These matrices are used for every scene object
  abcg::glUniformMatrix4fv(m_viewMatrixLocation, 1, GL_FALSE,
                           &m_camera.getViewMatrix()[0][0]);
  abcg::glUniformMatrix4fv(m_projMatrixLocation, 1, GL_FALSE,
                           &m_camera.getProjMatrix()[0][0]);

  abcg::glBindVertexArray(m_VAO);

  // Draw white bunny
  glm::mat4 model{1.0f};
  // renderizacao condicional caso nao tenha sido capturado
  if (m_pokemonCaptured[0] == false) {
    model = glm::translate(model, m_pokemonPosition[0]);
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.02f));

    abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE, &model[0][0]);
    abcg::glUniform4f(m_colorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
    abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT,
                         nullptr);
  }

  if (m_pokemonCaptured[1] == false) {
    // Draw yellow bunny
    model = glm::mat4(1.0);

    model = glm::translate(model, m_pokemonPosition[1]);
    model = glm::scale(model, glm::vec3(0.02f));

    abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE, &model[0][0]);
    abcg::glUniform4f(m_colorLocation, 1.0f, 0.8f, 0.0f, 1.0f);
    abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT,
                         nullptr);
  }

  if (m_pokemonCaptured[2] == false) {
    // Draw blue bunny
    model = glm::mat4(1.0);

    model = glm::translate(model, m_pokemonPosition[2]);
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.02f));

    abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE, &model[0][0]);
    abcg::glUniform4f(m_colorLocation, 0.0f, 0.8f, 1.0f, 1.0f);
    abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT,
                         nullptr);
  }

  // DRAW Pokeball
  if (m_pokeballLaunched == true) {

    abcg::glBindVertexArray(m_VAO_pokeball);

    // model = glm::mat4(1.0);
    glm::mat4 model_pokeball{1.0f};
    model_pokeball = glm::translate(model_pokeball, m_pokeballPosition);
    model_pokeball =
        glm::rotate(model_pokeball, glm::radians(90.0f), glm::vec3(0, 1, 0));
    model_pokeball = glm::scale(model_pokeball, glm::vec3(0.002f));

    abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE,
                             &model_pokeball[0][0]);
    abcg::glUniform4f(m_colorLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    abcg::glDrawElements(GL_TRIANGLES, m_indices_pokeball.size(),
                         GL_UNSIGNED_INT, nullptr);
  }

  abcg::glBindVertexArray(0);

  // Draw ground
  m_ground.paint();

  abcg::glUseProgram(0);
}

void Window::onPaintUI() { abcg::OpenGLWindow::onPaintUI(); }

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;
  m_camera.computeProjectionMatrix(size);
}

void Window::onDestroy() {
  m_ground.destroy();

  abcg::glDeleteProgram(m_program);
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}

void Window::onUpdate() {
  auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};

  // Update LookAt camera
  m_camera.dolly(m_dollySpeed * deltaTime);
  m_camera.truck(m_truckSpeed * deltaTime);
  m_camera.pan(m_panSpeed * deltaTime);

  // Atualiza a posição da Pokébola
  updatePokeballPosition();
}

void Window::launchPokeball() {
  if (!m_pokeballLaunched) {
    fmt::print("Pokebola vai!\n");

    m_pokeballPosition = m_camera.getEyePosition();

    glm::vec3 launchDirection =
        glm::normalize(m_camera.getLookAtPoint() - m_camera.getEyePosition());
    float launchSpeed = 2.0f;
    m_pokeballVelocity = launchDirection * launchSpeed;
    m_pokeballLaunched = true;
  }
}

void Window::updatePokeballPosition() {
  if (m_pokeballLaunched) {
    auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};

    m_pokeballPosition += m_pokeballVelocity * deltaTime;

    // Verifica se saiu da tela
    // ARRUMAR POKEBOLA QUANDO SAIR DA TELA
    if (m_pokeballPosition.x < -1.0f || m_pokeballPosition.x > 1.0f) {
      m_pokeballLaunched = false;
      fmt::print("Pokebola parou!\n");
    }

    // Verifica se colidiu com algum Pokémon
    // raio de colisao
    const float pokeballRadius = 0.1f;
    const float pokemonRadius = 0.5f;

    for (int i = 0; i < m_pokemonPosition->length(); ++i) {
      if (!m_pokemonCaptured[i]) {
        float distance =
            glm::distance(m_pokeballPosition, m_pokemonPosition[i]);

        if ((distance - pokemonRadius - pokeballRadius) < 0.02f) {
          // Colisão detectada
          fmt::print("Pokébola colidiu com Pokémon {}!\n", i + 1);
          // probabilidade de captura 45%
          std::uniform_real_distribution<float> rd_poke_capture(0.0f, 1.0f);

          if (rd_poke_capture(m_randomEngine) < 0.45f) {
            m_pokemonCaptured[i] = true;
            fmt::print("Pokémon {} capturado!\n", i + 1);
          } else {
            fmt::print("Pokémon {} escapou!\n", i + 1);
          }

          m_pokeballLaunched = false;
          break;
        }
      }
    }
  }
}
