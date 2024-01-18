#pragma once

#include <map>
#include <fstream>
#include <json.hpp>
#include <functional>

#define GUI_PATH "D:\\GUI\\"

using Json = nlohmann::json;

template<typename T>
void print(const T& t) {
	// write plateform specific print here
}

class Controller {
	using ControllerMethod = std::function<Json(Json)>;

	std::string name_;
	std::map<std::string, ControllerMethod> methods_;

	Controller(const std::string& name) : name_(name) {}

public:

	std::string getName() const {
		return name_;
	}

	void registerMethod(const std::string& methodName, const ControllerMethod& method) {
		methods_[methodName] = method;
	}

	void unregisterMethod(const std::string& methodName) {
		methods_.erase(methodName);
	}

	void invoke(const std::string& methodName, Json req) {
		if (methods_.find(methodName) == methods_.end()) {
			return;
		}

		auto output = methods_[methodName](req);

		std::ofstream outputFile(GUI_PATH + std::string("IPC.res.json"));
		outputFile << output.dump(2);
	}

	void invoke(const std::string& methodName) {
		std::ifstream file(GUI_PATH + std::string("IPC.req.json"));
		if (file) {
			Json input;
			file >> input;

			invoke(methodName, input);
		}
	}

	friend class ControllerManager;
};

class ControllerManager {
	static ControllerManager* instance_;
	std::vector<Controller*> controllers_;

	ControllerManager() = default;

public:
	static ControllerManager* Get() {
		if (!instance_) {
			instance_ = new ControllerManager;
		}
		return instance_;
	}

	Controller* createController(const std::string& controllerName) {
		auto controller = new Controller(controllerName);
		controllers_.push_back(controller);

		return controller;
	}

	void processRequest() {
		std::ifstream file(GUI_PATH + std::string("IPC.req.json"));
		if (!file) {
			print("IPC.req.json inexistant");
			return;
		}

		Json json;
		file >> json;

		try {
			std::string uri = json["uri"];
			auto dot = uri.find('.');
			if (dot == uri.npos) {
				print("Invalid request");
				return;
			}

			auto controllerName = uri.substr(0, dot);
			auto method = uri.substr(dot + 1);

			for (auto& controller : controllers_) {
				if (controller->getName() == controllerName) {
					controller->invoke(method);
					break;
				}
			}
		}
		catch (...) {}
	}
};

ControllerManager* ControllerManager::instance_ = nullptr;

void initCommunication() {
	static bool initialized = false;
	if (initialized) {
		return;
	}

	initialized = true;

	auto manager = ControllerManager::Get();

	// layer CONTROLLER

	auto controller = manager->createController("layer");

	controller->registerMethod("get_layer_names", [](Json json) -> Json {
		auto layers = getLayerList();
		Json res;

		for (auto& layer : layers) {
			res["layers"].push_back(acStrToStr(layer));
		}

		auto currLayer = getCurrentLayerName();
		res["current_layer"] = acStrToStr(currLayer);

		return res;
	});

	controller->registerMethod("set_layer", [controller](Json req) -> Json {
		try {
			auto& body = req["body"];
			if (body.is_null()) {
				print(controller->getName() + ".set_layer : null body received");
				return Json();
			}

			auto layerName = strToAcStr(req["body"]["layer"]);
			AcDbLayerTableRecord* layer = nullptr;

			getLayer(layer, layerName);
			if (!layer) {
				Json err;
				err["error"] = acStrToStr(layerName) + " does not exist";
				return err;
			}

			if (body.contains("active")) {
				setCurrentLayer(layer->id());
			}

			if (layer->upgradeOpen() == Acad::eOk) {
				if (body.contains("shown")) {
					layer->setIsHidden(!body["shown"]);
				}

				if (body.contains("frozen")) {
					layer->setIsFrozen(body["frozen"]);
				}

				if (body.contains("locked")) {
					layer->setIsLocked(body["locked"]);
				}
			}
			else {
				Json err;
				err["error"] = "Can not modify layer " + acStrToStr(layerName);
				return err;
			}

			layer->close();
		}
		catch (std::exception& e) {
			print(controller->getName() + ".set_layer : " + e.what());
		}
		return Json();
	});

	controller->registerMethod("get_layers", [controller](Json _) -> Json {
		resbuf buf;
		acedGetVar(_T("CLAYER"), &buf);

		AcString currLayer(buf.resval.rstring);
		auto layers = getLayerList();

		Json res;
		auto& layerList = res["layers"];

		for (auto& layerName : layers) {
			AcDbLayerTableRecord* layer = nullptr;

			getLayer(layer, layerName);
			if (!layer) {
				continue;
			}

			Json layerJson;

			layerJson["name"] = acStrToStr(layer->getName());
			layerJson["active"] = layer->getName() == currLayer;
			layerJson["shown"] = !layer->isHidden();
			layerJson["frozen"] = layer->isFrozen();
			layerJson["locked"] = layer->isLocked();
			
			layerList.push_back(layerJson);

			layer->close();
		}

		return res;
	});
}

void cmdCommunicateTauri() {
	initCommunication();

	auto manager = ControllerManager::Get();
	manager->processRequest();
}