import "package:flutter/services.dart" show rootBundle;

class DataPoint {
  final DateTime date;
  final double latitude;
  final double longitude;
  final double vibration;

  DataPoint(this.date, this.latitude, this.longitude, this.vibration);
}

class Simulator {
  final List<DataPoint> _data = [];

  Stream<DataPoint> get dataStream async* {
    if (_data.isEmpty) {
      await Future.delayed(const Duration(seconds: 1));
    }
    bool isFirst = true;
    DateTime lastDate = DateTime.now();
    for (var data in _data) {
      if (isFirst) {
        isFirst = false;
        yield data;
      } else {
        // Wait for the time difference
        final difference = data.date.difference(lastDate);
        await Future.delayed(difference * 0.3);
        // Yield the data
        yield data;
      }
      lastDate = data.date;
    }
  }

  Simulator() {
    // Read the file
    Future.microtask(() async {
      final text = await rootBundle.loadString('assets/sample_data.csv');
      final lines = text.split('\n');
      lines.removeAt(0);
      lines.removeLast();
      // Parse the data
      for (var line in lines) {
        final data = line.split(',');
        final date = int.parse(data[0]);
        final latitude = double.parse(data[2]);
        final longitude = double.parse(data[3]);
        final vibration = double.parse(data[1]);
        // Add the data to the list
        _data.add(DataPoint(
          DateTime.fromMillisecondsSinceEpoch(date * 1000),
          latitude,
          longitude,
          vibration,
        ));
      }
    });
  }
}
