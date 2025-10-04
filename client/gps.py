from dataclasses import dataclass
from math import cos, pi


@dataclass
class GlobalPosition():
    latitude: float
    longitude: float

    def toXY(self, center: 'GlobalPosition') -> tuple[float, float]:
        R = 6371000  # Radius of the Earth in meters

        lat_rad = (self.latitude - center.latitude) * (pi / 180.0)
        lon_rad = (self.longitude - center.longitude) * (pi / 180.0)

        x = R * lon_rad * cos(center.latitude * (pi / 180.0))
        y = R * lat_rad

        return (x, y)
