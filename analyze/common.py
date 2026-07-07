
from dataclasses import dataclass
import numpy as np

FIG_SIZE = np.array([8, 5])

@dataclass
class Scenario:
    type: str
    distance: float
    data: str
    color: str

    df: pd.DataFrame = None # type: ignore

    def name(self):
        return f"{self.type} {self.distance}m"