//
// SSAO
//   マルチレンダーターゲットを使った実装
//

#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/gl/gl.h>
#include <cinder/Camera.h>
#include <cinder/params/Params.h>
#include <cinder/Rand.h>
#include <functional>
#include <sstream>

#include "light.hpp"
#include "shader.hpp"
#include "model.hpp"

using namespace ci;
using namespace ci::app;


class SsaoApp : public App {
  enum {
    FBO_WIDTH  = 512,
    FBO_HEIGHT = 512,
  };
  
	gl::FboRef fbo;
  
  CameraPersp camera_persp;
  CameraOrtho camera_ui;

  float fov;
  float near_z;
  float far_z;

  Color diffuse;
  Color ambient;
  Color specular;
  vec3 direction;

  Light light;
  
  ivec2 mouse_prev_pos;
  int touch_num;

  quat rotate;
  vec3 translate;
  float z_distance;

  ShaderHolder shader_holder;
	ci::gl::UboRef ubo_light;
  
  Model model;
  vec3 offset;

  double prev_elapsed_time;

  bool do_animetion;
  bool no_animation;
  double current_animation_time;
  double animation_speed;

  bool do_disp_grid;
  float grid_scale;

  bool two_sided;
  bool disp_reverse;

  Color bg_color;
  gl::Texture2dRef bg_image;

  gl::GlslProgRef ao;

  gl::GlslProgRef df;
  float blurAmnt;
  float focusZ;

	gl::FboRef df_fbo;
  
  vec2 sampOffset[6];
  gl::Texture2dRef matrix_texture;

  
  std::string settings;

  
#if !defined (CINDER_COCOA_TOUCH)
  // iOS版はダイアログの実装が無い
	params::InterfaceGlRef params;
#endif

  
  float getVerticalFov();
  void setupCamera();
  void drawGrid();

  // ダイアログ関連
  void makeSettinsText();
  void createDialog();
  void drawDialog();


public:
  void setup() override;

  void resize() override;

  void fileDrop(FileDropEvent event) override;

  void mouseDown(MouseEvent event) override;
  void mouseDrag(MouseEvent event) override;
  void mouseWheel(MouseEvent event) override;

  void keyDown(KeyEvent event) override;

  void touchesBegan(TouchEvent event) override;
  void touchesMoved(TouchEvent event) override;
  void touchesEnded(TouchEvent event) override;

  void update() override;
  void draw() override;
};


// 垂直方向の視野角を計算する
//   縦長画面の場合は画面の縦横比から求める
float SsaoApp::getVerticalFov() {
  float aspect = ci::app::getWindowAspectRatio();
  camera_persp.setAspectRatio(aspect);

  if (aspect < 1.0) {
    // 画面が縦長になったら、幅基準でfovを求める
    // fovとnear_zから投影面の幅の半分を求める
    float half_w = std::tan(ci::toRadians(fov / 2)) * near_z;

    // 表示画面の縦横比から、投影面の高さの半分を求める
    float half_h = half_w / aspect;

    // 投影面の高さの半分とnear_zから、fovが求まる
    return toDegrees(std::atan(half_h / near_z) * 2);
  }
  else {
    // 横長の場合、fovは固定
    return fov;
  }
}



// 読み込んだモデルの大きさに応じてカメラを設定する
void SsaoApp::setupCamera() {
  // 初期位置はモデルのAABBの中心位置とする
  offset = -model.aabb.getCenter();

  // モデルがスッポリ画面に入るようカメラ位置を調整
  float w = length(model.aabb.getSize()) / 2.0f;
  float distance = w / std::tan(toRadians(fov / 2.0f));

  z_distance = distance;

  rotate    = glm::angleAxis(0.0f, vec3(0.0f, 1.0f, 0.0f));
  translate = vec3();

  // NearクリップとFarクリップを決める
  float size = length(model.aabb.getSize());
  near_z = size * 0.1f;
  far_z  = size * 100.0f;

  // グリッドのスケールも決める
  grid_scale = size / 5.0f;

  camera_persp.setNearClip(near_z);
  camera_persp.setFarClip(far_z);
}

// グリッド描画
void SsaoApp::drawGrid() {
  gl::ScopedGlslProg shader(gl::getStockShader(gl::ShaderDef().color()));

  gl::lineWidth(1.0f);

  for (int x = -5; x <= 5; ++x) {
    if (x == 0) gl::color(Color(1.0f, 0.0f, 0.0f));
    else        gl::color(Color(0.5f, 0.5f, 0.5f));

    gl::drawLine(vec3{ x * grid_scale, 0.0f, -5.0f * grid_scale }, vec3{ x * grid_scale, 0.0f, 5.0f * grid_scale });
  }
  for (int z = -5; z <= 5; ++z) {
    if (z == 0) gl::color(Color(0.0f, 0.0f, 1.0f));
    else        gl::color(Color(0.5f, 0.5f, 0.5f));

    gl::drawLine(vec3{ -5.0f * grid_scale, 0.0f, z * grid_scale }, vec3{ 5.0f * grid_scale, 0.0f, z * grid_scale });
  }

  gl::color(Color(0.0f, 1.0f, 0.0f));
  gl::drawLine(vec3{ 0.0f, -5.0f * grid_scale, 0.0f }, vec3{ 0.0f, 5.0f * grid_scale, 0.0f });
}


#if defined (CINDER_COCOA_TOUCH)

// iOS版はダイアログ関連の実装が無い
void SsaoApp::makeSettinsText() {}
void SsaoApp::createDialog() {}
void SsaoApp::drawDialog() {}

#else

// 現在の設定をテキスト化
void SsaoApp::makeSettinsText() {
  std::ostringstream str;

  str << (two_sided    ? "D" : " ") << " "
      << (do_animetion ? "A" : " ") << " "
      << (no_animation ? "M" : " ") << " "
      << (disp_reverse ? "F" : " ");

  settings = str.str();
  params->removeParam("Settings");
  params->addParam("Settings", &settings, true);
}

// ダイアログ作成
void SsaoApp::createDialog() {
	// 各種パラメーター設定
	params = params::InterfaceGl::create("Preview params", toPixels(ivec2(200, 400)));

  params->addParam("FOV", &fov).min(1.0f).max(180.0f).updateFn([this]() {
      camera_persp.setFov(fov);
    });

  params->addSeparator();

  params->addParam("BG", &bg_color);

  params->addSeparator();

  params->addParam("Ambient",   &ambient).updateFn([this](){ light.setAmbient(ambient); });
  params->addParam("Diffuse",   &diffuse).updateFn([this](){ light.setDiffuse(diffuse); });
  params->addParam("Speculat",  &specular).updateFn([this](){ light.setSpecular(specular); });
  params->addParam("Direction", &direction).updateFn([this](){ light.setDirection(direction); });

  params->addSeparator();

  params->addParam("Speed", &animation_speed).min(0.1).max(10.0).precision(2).step(0.05);

  makeSettinsText();

  params->addSeparator();

  params->addParam("blurAmnt", &blurAmnt).min(0.0).max(1.0).step(0.001);
  params->addParam("focusZ", &focusZ).min(0.0).max(1.0).step(0.001);
  
}

// ダイアログ表示
void SsaoApp::drawDialog() {
	params->draw();
}

#endif


void SsaoApp::setup() {
#if defined (CINDER_COCOA_TOUCH)
  // 縦横画面両対応
  getSignalSupportedOrientations().connect([]() { return ci::app::InterfaceOrientation::All; });
#endif

  gl::enableVerticalSync(true);

  // FBO生成
  {
    auto format = gl::Fbo::Format()
      // depth
      .depthBuffer()
      .depthTexture()
      // albedo
      .attachment(GL_COLOR_ATTACHMENT0, gl::Texture2d::create(FBO_WIDTH, FBO_HEIGHT))
      // normal
      .attachment(GL_COLOR_ATTACHMENT1, gl::Texture2d::create(FBO_WIDTH, FBO_HEIGHT))
      // AO
      .attachment(GL_COLOR_ATTACHMENT2,
                  gl::Texture2d::create(FBO_WIDTH, FBO_HEIGHT,
                                        gl::Texture2d::Format()
                                        .internalFormat(GL_RGB16F)
                                        .dataType(GL_FLOAT)
                                        .wrap(GL_REPEAT)
                                        .minFilter(GL_NEAREST)
                                        .magFilter(GL_NEAREST)))
      ;
    
    fbo = gl::Fbo::create(FBO_WIDTH, FBO_HEIGHT, format);
  }

  {
    auto format = gl::Fbo::Format()
      .colorTexture()
      ;

    df_fbo = gl::Fbo::create(FBO_WIDTH, FBO_HEIGHT, format);
  }
  
  
  touch_num = 0;
  // アクティブになった時にタッチ情報を初期化
  getSignalDidBecomeActive().connect([this](){ touch_num = 0; });

  // モデルデータ読み込み
  model = loadModel(getAssetPath("chr_old.obj").string());
  loadShader(shader_holder, model);
  
  prev_elapsed_time = 0.0;

  do_animetion = true;
  no_animation = false;
  current_animation_time = 0.0f;
  animation_speed = 1.0f;

  do_disp_grid = true;
  grid_scale = 1.0f;

  // カメラの設定
  fov = 35.0f;
  setupCamera();

  camera_persp = CameraPersp(FBO_WIDTH, FBO_HEIGHT,
                             fov,
                             near_z, far_z);

  camera_persp.setEyePoint(vec3());
  camera_persp.setViewDirection(vec3{ 0.0f, 0.0f, -1.0f });

  rotate = glm::angleAxis(0.0f, vec3(0.0f, 1.0f, 0.0f));

  // UI用カメラ
  camera_ui = CameraOrtho(0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 5.0f);
  camera_ui.setEyePoint(vec3());
  camera_ui.setViewDirection(vec3{ 0.0f, 0.0f, -1.0f });

  // ライトの設定
  ambient   = Color(0.3, 0.3, 0.3);
  diffuse   = Color(0.9, 0.9, 0.9);
  specular  = Color(0.5, 0.5, 0.5);
  direction = vec3(0, 0, 1);
  
  light.setAmbient(ambient);
  light.setDiffuse(diffuse);
  light.setSpecular(specular);
  light.setDirection(direction);

  // UBOを使い複数のシェーダーで値を共有
  ubo_light = gl::Ubo::create(sizeof (Light), &light);
	ubo_light->bindBufferBase(0);

  
  bg_color = Color(0.7f, 0.7f, 0.7f);
  bg_image = gl::Texture2d::create(loadImage(loadAsset("bg2.png")));

  two_sided    = false;
  disp_reverse = false;

  {
    auto shader = readShader("ao", "ao2");
    ao = ci::gl::GlslProg::create(shader.first, shader.second);
    ao->uniform("uTex0", 0);
    ao->uniform("uTex1", 1);
    ao->uniform("uTex2", 2);
    ao->uniform("matrix_texture", 3);

    for (u_int i = 0; i < 6; ++i) {
      sampOffset[i] = vec2(Rand::randFloat(-0.01f, 0.01f),
                           Rand::randFloat(-0.01f, 0.01f));
    }
    ao->uniform("sampOffset", sampOffset, 6);

    std::vector<mat2> rotations;
    for (size_t i = 0; i < 16; ++i) {
      float r = Rand::randFloat(-M_PI, M_PI);
      rotations.push_back(mat2(std::cos(r), -std::sin(r), std::sin(r), std::cos(r)));
    }
    
    matrix_texture = gl::Texture2d::create((void*)&rotations[0],
                                           GL_RGBA,
                                           4, 4,
                                           gl::Texture2d::Format()
                                           .internalFormat(GL_RGBA16F)
                                           .dataType(GL_FLOAT)
                                           .wrap(GL_REPEAT)
                                           .minFilter(GL_NEAREST)
                                           .magFilter(GL_NEAREST));
  }

  {
    auto shader = readShader("ao", "df");
    df = ci::gl::GlslProg::create(shader.first, shader.second);
    df->uniform("uTex0", 0);
    df->uniform("uTex1", 1);
    blurAmnt = 0.0f;
    focusZ = 0.5f;
  }
  
  // ダイアログ作成
  createDialog();
  
  gl::enableAlphaBlending();
  gl::enable(GL_CULL_FACE);
}


void SsaoApp::resize() {
  camera_persp.setFov(getVerticalFov());
  touch_num = 0;
}


void SsaoApp::fileDrop(FileDropEvent event) {
  const auto& path = event.getFiles();
  console() << "Load: " << path[0] << std::endl;

  model = loadModel(path[0].string());
  loadShader(shader_holder, model);

  // 読み込んだモデルがなんとなく中心に表示されるよう調整
  offset = -model.aabb.getCenter();

  // FIXME:モデルのAABBを計算する時にアニメーションを適用している
  //       そのままだとアニメーションの情報が残ってしまっているので
  //       一旦リセット
  if (no_animation) resetModelNodes(model);

  setupCamera();
  current_animation_time = 0.0;
  touch_num = 0;
  disp_reverse = false;
}


void SsaoApp::mouseDown(MouseEvent event) {
  if (touch_num > 1) return;

  if (event.isLeft()) {
    // TIPS:マウスとワールド座標で縦方向の向きが逆
    auto pos = event.getPos();
    mouse_prev_pos = pos;
  }
}

void SsaoApp::mouseDrag(MouseEvent event) {
  if (touch_num > 1) return;

  if (!event.isLeftDown()) return;

  auto mouse_pos = event.getPos();

  if (event.isShiftDown()) {
	  auto d = mouse_pos - mouse_prev_pos;
	  vec3 v(d.x, -d.y, 0.0f);

	  float t = std::tan(toRadians(fov) / 2.0f) * z_distance;
	  translate += v * t * 0.004f;
  }
  else if (event.isControlDown()) {
	  float d = mouse_pos.y - mouse_prev_pos.y;

	  float t = std::tan(toRadians(fov) / 2.0f) * z_distance;
	  z_distance = std::max(z_distance - d * t * 0.008f, near_z);
  }
  else {
    vec2 d(mouse_pos - mouse_prev_pos);
    float l = length(d);
    if (l > 0.0f) {
      d = normalize(d);
      vec3 v(d.y, d.x, 0.0f);
      quat r = glm::angleAxis(l * 0.01f, v);
      rotate = r * rotate;
    }

  }
  mouse_prev_pos = mouse_pos;
}


void SsaoApp::mouseWheel(MouseEvent event) {
  // OSX:マルチタッチ操作の時に呼ばれる
  if (touch_num > 1) return;

	// 距離に応じて比率を変える
	float t = std::tan(toRadians(fov) / 2.0f) * z_distance;
	z_distance = std::max(z_distance + event.getWheelIncrement() * t * 0.5f, near_z);
}


void SsaoApp::keyDown(KeyEvent event) {
  int key_code = event.getCode();
  switch (key_code) {
  case KeyEvent::KEY_r:
    {
      animation_speed = 1.0;
      do_animetion = true;
      no_animation = false;

      setupCamera();
      touch_num = 0;
    }
    break;

  case KeyEvent::KEY_SPACE:
    {
      do_animetion = !do_animetion;
      makeSettinsText();
    }
    break;

  case KeyEvent::KEY_m:
    {
      no_animation = !no_animation;
      if (no_animation) {
        resetModelNodes(model);
      }
      makeSettinsText();
    }
    break;

  case KeyEvent::KEY_g:
    {
      do_disp_grid = !do_disp_grid;
    }
    break;

  case KeyEvent::KEY_d:
    {
      two_sided = !two_sided;
      two_sided ? gl::disable(GL_CULL_FACE)
                : gl::enable(GL_CULL_FACE);

      makeSettinsText();
    }
    break;

  case KeyEvent::KEY_f:
    {
      reverseModelNode(model);
      disp_reverse = !disp_reverse;
      makeSettinsText();
    }
    break;


  case KeyEvent::KEY_PERIOD:
    {
      animation_speed = std::min(animation_speed * 1.25, 10.0);
      console() << "speed:" << animation_speed << std::endl;
      makeSettinsText();
    }
    break;

  case KeyEvent::KEY_COMMA:
    {
      animation_speed = std::max(animation_speed * 0.95, 0.1);
      console() << "speed:" << animation_speed << std::endl;
      makeSettinsText();
    }
    break;


  }
}


void SsaoApp::touchesBegan(TouchEvent event) {
  const auto& touches = event.getTouches();

  touch_num += touches.size();
}

void SsaoApp::touchesMoved(TouchEvent event) {
//  if (touch_num < 2) return;

  const auto& touches = event.getTouches();

#if defined (CINDER_COCOA_TOUCH)
  if (touch_num == 1) {
    vec2 d{ touches[0].getPos() -  touches[0].getPrevPos() };
    float l = length(d);
    if (l > 0.0f) {
      d = normalize(d);
      vec3 v(d.y, d.x, 0.0f);
      quat r = glm::angleAxis(l * 0.01f, v);
      rotate = r * rotate;
    }

    return;
  }
#endif
  if (touches.size() < 2) return;

  vec3 v1{ touches[0].getX(), -touches[0].getY(), 0.0f };
  vec3 v2{ touches[1].getX(), -touches[1].getY(), 0.0f };
  vec3 v1_prev{ touches[0].getPrevX(), -touches[0].getPrevY(), 0.0f };
  vec3 v2_prev{ touches[1].getPrevX(), -touches[1].getPrevY(), 0.0f };

  vec3 d = v1 - v1_prev;

  float l = length(v2 - v1);
  float l_prev = length(v2_prev - v1_prev);
  float ld = l - l_prev;

  // 距離に応じて比率を変える
  float t = std::tan(toRadians(fov) / 2.0f) * z_distance;

  if (std::abs(ld) < 3.0f) {
    translate += d * t * 0.005f;
  }
  else {
    z_distance = std::max(z_distance - ld * t * 0.01f, 0.01f);
  }
}

void SsaoApp::touchesEnded(TouchEvent event) {
  const auto& touches = event.getTouches();

  // 最悪マイナス値にならないよう
  touch_num = std::max(touch_num - int(touches.size()), 0);
}


void SsaoApp::update() {
  double elapsed_time = getElapsedSeconds();
  double delta_time   = elapsed_time - prev_elapsed_time;

  if (do_animetion && !no_animation) {
    current_animation_time += delta_time * animation_speed;
    updateModel(model, current_animation_time, 0);
  }

  prev_elapsed_time = elapsed_time;
}

void SsaoApp::draw() {
  {
    gl::ScopedViewport viewportScope(ivec2(0), fbo->getSize());
    gl::ScopedFramebuffer fboScope(fbo);
    
    gl::clear(ColorA(0.0f, 0.0f, 0.0f));

    {
      // 背景描画
      gl::setMatrices(camera_ui);

      gl::disableAlphaBlending();
      gl::disableDepthRead();
      gl::disableDepthWrite();

      gl::color(bg_color);
      gl::translate(0.0f, 0.0f, -2.0f);
      gl::draw(bg_image, Rectf{ 0.0f, 0.0f, 1.0f, 1.0f });
    }
    
    // モデル描画
    gl::setMatrices(camera_persp);

    gl::enableAlphaBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();

    gl::translate(vec3(0, 0.0, -z_distance));
    gl::translate(translate);
    gl::rotate(rotate);

    gl::translate(offset);

    ubo_light->copyData(sizeof (Light), &light);
  
    drawModel(model, shader_holder);

#if !defined (CINDER_COCOA_TOUCH)
    // FIXME:iOSだと劇重
    if (do_disp_grid) drawGrid();
#endif
  }


#if 1
  {
    // Depth of Field(2-pass)
    gl::ScopedViewport viewportScope(ivec2(0), fbo->getSize());
    gl::ScopedFramebuffer fboScope(df_fbo);
    gl::ScopedGlslProg shader(df);

    gl::setMatrices(camera_ui);

    gl::disableAlphaBlending();
    gl::disableDepthRead();
    gl::disableDepthWrite();

    fbo->getTexture2d(GL_COLOR_ATTACHMENT0)->bind(0);
    fbo->getDepthTexture()->bind(1);
    df->uniform("blurAmnt", vec2(blurAmnt, 0.0));
    df->uniform("focusZ",   focusZ);

    gl::drawSolidRect(Rectf{ -1.0f, 1.0f, 1.0f, -1.0f });
  }

  {
    gl::ScopedGlslProg shader(df);
    
    df_fbo->getColorTexture()->bind(0);
    df->uniform("blurAmnt", vec2(0.0, blurAmnt));
    
    gl::drawSolidRect(Rectf{ -1.0f, 1.0f, 1.0f, -1.0f });
  }
#else
  {
    // SSAO
    gl::ScopedGlslProg shader(ao);

    fbo->getTexture2d(GL_COLOR_ATTACHMENT0)->bind(0);
    fbo->getTexture2d(GL_COLOR_ATTACHMENT1)->bind(1);
    fbo->getDepthTexture()->bind(2);
    matrix_texture->bind(3);
    
    gl::drawSolidRect(Rectf{ -1.0f, 1.0f, 1.0f, -1.0f });
  }
#endif

  // ダイアログ表示
   drawDialog();
}


// FIXME:なんかいくない
enum {
  WINDOW_WIDTH  = 800,
  WINDOW_HEIGHT = 600,

#if defined (CINDER_COCOA_TOUCH)
  MSAA_VALUE = 4,
#else
  MSAA_VALUE = 8,
#endif
};

// アプリのラウンチコード
CINDER_APP(SsaoApp,
           RendererGl,
           [](App::Settings* settings) {
             // 画面サイズを変更する
             settings->setWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
             // Retinaディスプレイ有効
             settings->setHighDensityDisplayEnabled(true);
             
             // マルチタッチ有効
             settings->setMultiTouchEnabled(true);
           })
