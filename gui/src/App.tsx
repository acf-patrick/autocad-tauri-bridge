import { useEffect, useState } from "react";
import "./App.css";
import { invoke } from "@tauri-apps/api/tauri";

type Layer = {
  name: string;
  shown: boolean;
  locked: boolean;
  frozen: boolean;
  active: boolean;
};

if (import.meta.env.PROD) {
  document.addEventListener("contextmenu", (e) => e.preventDefault());
}

function call(uri: string, payload?: any) {
  return invoke("call", {
    uri,
    payload: JSON.stringify(payload),
  });
}

function App() {
  const [layers, setLayers] = useState<Layer[]>([]);

  const fetchLayers = async () => {
    const res = await call("layer.get_layers");

    const payload: {
      layers: Layer[];
    } = JSON.parse(res as string);

    setLayers(payload.layers);
  };

  useEffect(() => {
    fetchLayers().catch((err) => {
      console.error(err);
    });
  }, []);

  const setLayerState = (layer: string, state: string, value: boolean) => {
    let payload: any = {
      layer,
    };
    payload[state] = value;

    const setLayer = async () => {
      await call("layer.set_layer", payload);
      await fetchLayers();
    };

    setLayer().catch((err) => console.error(err));
  };

  return (
    <>
      <p>Layer list : </p>
      <table>
        <thead>
          <tr>
            <th>Active</th>
            <th>Name</th>
            <th>Shown</th>
            <th>Frozen</th>
            <th>Locked</th>
          </tr>
        </thead>
        <tbody>
          {layers.map((layer, i) => (
            <tr key={i}>
              <td>
                <input
                  type="checkbox"
                  checked={layer.active}
                  onChange={(e) => {
                    if (e.currentTarget.checked)
                      setLayerState(
                        layer.name,
                        "active",
                        e.currentTarget.checked
                      );
                  }}
                />
              </td>
              <td className="layer_name">{layer.name}</td>
              <td>
                <input
                  type="checkbox"
                  defaultChecked={layer.shown}
                  onChange={(e) =>
                    setLayerState(layer.name, "shown", e.currentTarget.checked)
                  }
                />
              </td>
              <td>
                <input
                  type="checkbox"
                  defaultChecked={layer.frozen}
                  onChange={(e) =>
                    setLayerState(layer.name, "frozen", e.currentTarget.checked)
                  }
                />
              </td>
              <td>
                <input
                  type="checkbox"
                  defaultChecked={layer.locked}
                  onChange={(e) =>
                    setLayerState(layer.name, "locked", e.currentTarget.checked)
                  }
                />
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </>
  );
}

export default App;
