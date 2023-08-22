import 'package:diagnostics_app/simulator.dart';
import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:flutter_map/plugin_api.dart';
import 'package:latlong2/latlong.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Diagnostics In Car',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.lime),
        useMaterial3: true,
      ),
      home: const MyHomePage(title: 'Flutter Demo Home Page'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  static const int _maxDataPoints = 20;

  Simulator simulator = Simulator();
  GlobalKey<FlutterMapState> mapKey = GlobalKey<FlutterMapState>();

  List<double> _vibrationLevels = [];
  double maxVibration = 0;
  LatLng currentLocation = const LatLng(0, 0);

  List<LatLng> _route = [];

  @override
  void initState() {
    super.initState();
    simulator.dataStream.listen((event) {
      setState(() {
        _vibrationLevels.add(event.vibration);
        if (event.vibration > maxVibration) {
          maxVibration = event.vibration;
        }
        if (_vibrationLevels.length > _maxDataPoints) {
          _vibrationLevels.removeAt(0);
        }
        currentLocation = LatLng(event.latitude, event.longitude);
        _route.add(currentLocation);
        mapKey.currentState?.move(
          currentLocation,
          18.0,
          source: MapEventSource.custom,
        );
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    if (_vibrationLevels.isEmpty) {
      return const Center(child: CircularProgressIndicator());
    }
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: Text("Diagnostics In Car"),
      ),
      body: Stack(
        children: [
          FlutterMap(
            key: mapKey,
            options: MapOptions(
              zoom: 13.0,
              maxZoom: 18.0,
            ),
            children: [
              TileLayer(
                urlTemplate: 'https://tile.openstreetmap.org/{z}/{x}/{y}.png',
                userAgentPackageName: 'com.example.app',
              ),
              PolylineLayer(
                polylines: [
                  Polyline(
                    points: _route,
                    strokeWidth: 6.0,
                    color: Colors.red.shade600,
                  ),
                ],
              )
            ],
          ),
          Center(
            child: Container(
              width: 20,
              height: 20,
              decoration: BoxDecoration(
                color: Colors.red.shade600,
                shape: BoxShape.circle,
              ),
            ),
          ),
          Align(
            alignment: Alignment.topLeft,
            child: Container(
              padding: const EdgeInsets.all(30.0),
              width: 600 * 0.8,
              height: 400 * 0.8,
              child: Card(
                child: Padding(
                  padding: const EdgeInsets.all(20.0),
                  child: Column(
                    children: [
                      Text(
                        "Vibration level",
                        style: Theme.of(context).textTheme.titleMedium,
                      ),
                      const SizedBox(height: 20),
                      Expanded(
                        child: LineChart(
                          LineChartData(
                            minX: 0,
                            maxX: _maxDataPoints - 1,
                            minY: 0,
                            maxY: maxVibration,
                            lineBarsData: [
                              LineChartBarData(
                                isCurved: true,
                                color: Colors.lime.shade400,
                                barWidth: 8,
                                isStrokeCapRound: true,
                                dotData: const FlDotData(show: false),
                                belowBarData: BarAreaData(
                                  show: true,
                                  color: Colors.lime.shade400.withOpacity(0.3),
                                ),
                                spots: _vibrationLevels.indexed
                                    .map<FlSpot>(
                                      (e) => FlSpot(e.$1.toDouble(), e.$2),
                                    )
                                    .toList(),
                              )
                            ],
                            gridData: const FlGridData(
                              show: false,
                            ),
                            borderData: FlBorderData(
                              show: false,
                            ),
                            titlesData: const FlTitlesData(
                              show: false,
                            ),
                          ),
                          duration: Duration.zero,
                        ),
                      ),
                    ],
                  ),
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}
