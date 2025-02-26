﻿
#include "Title.h"
#include "../../Input/Input.hpp"
#include "SceneManager.hpp"
#include "../GameController.h"
#include "../src/Utility/Parameter.hpp"
#include "../../Utility/Math.hpp"
//#include <filesystem>
namespace Scene
{
	using namespace ECS;

	namespace
	{
		struct Sphere
		{
			JsonRead json;

			Vec3 pos;
			float r = 0;
			Sphere()
			{
				json.load("data.json");
			}
			void draw()
			{
				MATERIALPARAM MatParam;
				auto d_r = json.getParameter<float>("mat", "Diffuse", 0);
				auto d_g = json.getParameter<float>("mat", "Diffuse", 1);
				auto d_b = json.getParameter<float>("mat", "Diffuse", 2);
				auto d_a = json.getParameter<float>("mat", "Diffuse", 3);

				auto a_r = json.getParameter<float>("mat", "Ambient", 0);
				auto a_g = json.getParameter<float>("mat", "Ambient", 1);
				auto a_b = json.getParameter<float>("mat", "Ambient", 2);
				auto a_a = json.getParameter<float>("mat", "Ambient", 3);

				auto s_r = json.getParameter<float>("mat", "Specular", 0);
				auto s_g = json.getParameter<float>("mat", "Specular", 1);
				auto s_b = json.getParameter<float>("mat", "Specular", 2);
				auto s_a = json.getParameter<float>("mat", "Specular", 3);

				auto e_r = json.getParameter<float>("mat", "Emissive", 0);
				auto e_g = json.getParameter<float>("mat", "Emissive", 1);
				auto e_b = json.getParameter<float>("mat", "Emissive", 2);
				auto e_a = json.getParameter<float>("mat", "Emissive", 3);

				MatParam.Diffuse = GetColorF(d_r, d_g, d_b, d_a);	// デフューズカラー
				MatParam.Ambient = GetColorF(a_r, a_g, a_b, a_a);	// アンビエントカラー
				MatParam.Specular = GetColorF(s_r, s_g, s_b, s_a);	// スペキュラカラー
				MatParam.Emissive = GetColorF(e_r, e_g, e_b, e_a);	// エミッシブカラー( 自己発光 )
				MatParam.Power = json.getParameter<float>("mat", "Power");						// スペキュラの強さ

				// マテリアルのパラメータをセット
				SetMaterialParam(MatParam);
				DrawSphere3D(
					pos.getVector<VECTOR>(),
					r,
					32,
					GetColor(int(d_r * 255), (int(d_g * 255)), (int(d_b * 255))),
					GetColor(int(d_r * 255), (int(d_g * 255)), (int(d_b * 255))),
					true);
			}
		};
		Vec3 velocity;
		Vec3 planeNormal{ 0,1,0 };//床
		Sphere s;

		struct Cube
		{
			JsonRead json{};
			Vec3 pos;
			Vec3 scale;
			Cube()
			{
				json.load("data.json");
			}
			void draw()
			{
				//posをキューブの中心として描画する
				auto radius = scale / 2;
				auto start = pos - radius;
				auto end = pos + radius;

				MATERIALPARAM MatParam;
				auto d_r = json.getParameter<float>("mat", "Diffuse", 0);
				auto d_g = json.getParameter<float>("mat", "Diffuse", 1);
				auto d_b = json.getParameter<float>("mat", "Diffuse", 2);
				auto d_a = json.getParameter<float>("mat", "Diffuse", 3);

				auto a_r = json.getParameter<float>("mat", "Ambient", 0);
				auto a_g = json.getParameter<float>("mat", "Ambient", 1);
				auto a_b = json.getParameter<float>("mat", "Ambient", 2);
				auto a_a = json.getParameter<float>("mat", "Ambient", 3);

				auto s_r = json.getParameter<float>("mat", "Specular", 0);
				auto s_g = json.getParameter<float>("mat", "Specular", 1);
				auto s_b = json.getParameter<float>("mat", "Specular", 2);
				auto s_a = json.getParameter<float>("mat", "Specular", 3);

				auto e_r = json.getParameter<float>("mat", "Emissive", 0);
				auto e_g = json.getParameter<float>("mat", "Emissive", 1);
				auto e_b = json.getParameter<float>("mat", "Emissive", 2);
				auto e_a = json.getParameter<float>("mat", "Emissive", 3);

				MatParam.Diffuse = GetColorF(d_r, d_g, d_b, d_a);	// デフューズカラー
				MatParam.Ambient = GetColorF(a_r, a_g, a_b, a_a);	// アンビエントカラー
				MatParam.Specular = GetColorF(s_r, s_g, s_b, s_a);	// スペキュラカラー
				MatParam.Emissive = GetColorF(e_r, e_g, e_b, e_a);	// エミッシブカラー( 自己発光 )
				MatParam.Power = json.getParameter<float>("mat", "Power");						// スペキュラの強さ

				// マテリアルのパラメータをセット
				SetMaterialParam(MatParam);
				DrawCube3D(
					start.getVector<VECTOR>(),
					end.getVector<VECTOR>(),
					GetColor(int(d_r * 255), (int(d_g * 255)), (int(d_b * 255))),
					GetColor(int(d_r * 255), (int(d_g * 255)), (int(d_b * 255))),
					true);
			}
		};
		Cube c[5];

		const Vec3 NORMALS[6] =
		{
			Vec3{0,1,0},
			Vec3{0,-1,0},
			Vec3{-1,0,0},
			Vec3{1,0,0},
			Vec3{0,0,-1},
			Vec3{0,0,1}
		};

		static const Vec2& GetWallScratchVector(Vec2& velocity, Vec2& normal)
		{
			Vec2 nom = velocity - normal * Vec2::Dot(velocity, normal);
			velocity = nom.normalize();
			return velocity;
		}
		Entity* circle;
		Entity* l;
		Entity* l2;
		Vec2 dir{ 1,0 };//向き
		float speed = 5;
	}

	Title::~Title()
	{

	}
	Title::Title(IOnSceneChangeCallback* sceneTitleChange, ECS::EntityManager* entityManager)
		: AbstractScene(sceneTitleChange)
		, entityManager_(entityManager)
	{
	}

	void Title::initialize()
	{
		//ディレクショナルライトの設定
		CreateDirLightHandle(VGet(0.0f, 1.0f, 0.0f));
		CreateDirLightHandle(VGet(0.0f, -1.0f, 0.0f));
		CreateDirLightHandle(VGet(1.0f, 0.0f, 0.0f));
		CreateDirLightHandle(VGet(-1.0f, 0.0f, 0.0f));
		CreateDirLightHandle(VGet(0.0f, 0.0f, 1.0f));
		CreateDirLightHandle(VGet(0.0f, 0.0f, -1.0f));

		//カメラ設定
		SetCameraNearFar(0.1f, 10000.0f);

		s.pos = Vec3{ 0.f,0.f,0.f };
		s.r = 5;
		c[0].pos = Vec3{ 0,-50,0 };
		c[0].scale = Vec3{ 100,5,100 };

		c[1].pos = Vec3{ 0,60,0 };
		c[1].scale = Vec3{ 100,5,100 };

		c[2].pos = Vec3{ 56,0,0 };
		c[2].scale = Vec3{ 5,100,100 };

		c[3].pos = Vec3{ -70,0,0 };
		c[3].scale = Vec3{ 5,100,100 };

		c[4].pos = Vec3{ 0,0,70 };
		c[4].scale = Vec3{ 100,100,5 };

		//速度ベクトル
		velocity.z = 1;
		velocity.y = -1;
		velocity.x = -1;
		ResourceManager::GetGraph().load("Resource/image/bb.png","p");

		circle = ECS::Primitive2D::CreateCircle(Vec2{ 30.f,640.f }, 20.f, *entityManager_);
		circle->addComponent<Physics2D>();
		l = ECS::Primitive2D::CreateLine(Vec2{ 0.f,720.f }, Vec2{ 900.f,300.f }, *entityManager_);
		l2 = ECS::Primitive2D::CreateLine(Vec2{ 900.f,300.f }, Vec2{ 1280.f,300.f }, *entityManager_);
	}

	void Title::update()
	{
		if (Input::Get().getKeyFrame(KEY_INPUT_RIGHT) >= 1)
		{
			speed = 5;
			dir.x = 1;
		}
		if (Input::Get().getKeyFrame(KEY_INPUT_LEFT) >= 1)
		{
			speed = 5;
			dir.x = -1;
		}
		if (Input::Get().getKeyFrame(KEY_INPUT_UP) == 1)
		{
			speed = 5;
			circle->getComponent<Velocity2D>().val.y += 16;
		}
		if (!Input::Get().getIsAnyInput())
		{
			speed = 0.0000001f;
		}
		
		circle->getComponent<Velocity2D>().val.x = dir.x * speed;
		
		if (Collision2D::CirecleAndLine(circle, l))
		{
			auto n = Math::GetLineNormal(l->getComponent<LineData2D>().p1, l->getComponent<LineData2D>().p2);
			circle->getComponent<Velocity2D>().val = GetWallScratchVector(circle->getComponent<Velocity2D>().val, n);
			DOUT << circle->getComponent<Velocity2D>().val.x << ":::::"<< circle->getComponent<Velocity2D>().val.y << std::endl;
			circle->getComponent<Velocity2D>().val *= speed;
			circle->getComponent<Gravity>().val = 0;
		}
		else
		{
			circle->getComponent<Physics2D>().setGravity();
		}

		if (Collision2D::CirecleAndLine(circle, l2))
		{
			auto n = Math::GetLineNormal(l2->getComponent<LineData2D>().p1, l2->getComponent<LineData2D>().p2);
			circle->getComponent<Velocity2D>().val = GetWallScratchVector(circle->getComponent<Velocity2D>().val, n);
			circle->getComponent<Velocity2D>().val *= speed;
			circle->getComponent<Gravity>().val = 0;
		}
		//else
		//{
			//circle->getComponent<Physics2D>().setGravity();
		//}
		


		
		//for (size_t i = 0; i < std::size(c); ++i)
		//{
		//	//if (s.pos.getDistanceToPlain(c[i].pos, NORMALS[i]) <= 5)
		//	{
		//	//	velocity.calcWallScratchVector(NORMALS[i]);
		//		//velocity.calcReflection(NORMALS[i]);
		//	}
		//}
		//if (s.pos.getDistanceToPlain(c[0].pos, NORMALS[0]) <= 5)
		//{
		//	velocity.calcWallScratchVector(NORMALS[0]);
		//	
		//}
		////手前(透明な壁)
		//if (s.pos.getDistanceToPlain(Vec3{ 0,0,-80 }, NORMALS[5]) <= 5)
		//{
		//	velocity.calcReflection(NORMALS[5]);
		//}

		//s.pos += velocity * 1;
		//static float x = 0, y = 0, z = -90;
		//if (Input::Get().getKeyFrame(KEY_INPUT_UP) > 0) { ++z; }
		//if (Input::Get().getKeyFrame(KEY_INPUT_DOWN) > 0) { --z; }
		//if (Input::Get().getKeyFrame(KEY_INPUT_LEFT) > 0) { --x; }
		//if (Input::Get().getKeyFrame(KEY_INPUT_RIGHT) > 0) { ++x; }
		//if (Input::Get().getKeyFrame(KEY_INPUT_Q) > 0) { ++y; }
		//if (Input::Get().getKeyFrame(KEY_INPUT_E) > 0) { --y; }
		//SetCameraPositionAndAngle(VGet(x, y, z), Math::ToRadian(0.f), 0.0f, 0.0f);
		entityManager_->update();
	}

	void Title::draw()
	{
		SetDrawMode(DX_DRAWMODE_BILINEAR);
		//グループ順に描画
		entityManager_->orderByDraw(ENTITY_GROUP::MAX);

		//s.draw();
		//for (auto& it : c) { it.draw(); }
		//entityManager_->draw3D();
		SetDrawMode(DX_DRAWMODE_NEAREST);
	}

}