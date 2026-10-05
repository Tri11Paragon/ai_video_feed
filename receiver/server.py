import os
from contextlib import contextmanager, asynccontextmanager

import mysql.connector
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel, Field

TABLE_NAME = "device_count_events"

class DeviceCountPayload(BaseModel):
    device: str = Field(..., min_length=1, max_length=255)
    person: int
    vehicle: int
    person_delta: int
    vehicle_delta: int


@contextmanager
def get_db_connection():
    conn = mysql.connector.connect(
        host=os.environ.get("MYSQL_HOST", "localhost"),
        port=int(os.environ.get("MYSQL_PORT", "3306")),
        user=os.environ["MYSQL_USER"],
        password=os.environ["MYSQL_PASSWORD"],
        database=os.environ["MYSQL_DATABASE"],
    )

    try:
        yield conn
    finally:
        conn.close()


def initialize_database():
    create_table_sql = f"""
    CREATE TABLE IF NOT EXISTS {TABLE_NAME} (
        id BIGINT NOT NULL AUTO_INCREMENT,
        time_stamp TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
        device_name VARCHAR(64) NOT NULL,
        person_delta INT NOT NULL,
        vehicle_delta INT NOT NULL,

        PRIMARY KEY (id),
        INDEX idx_time_stamp (time_stamp),
        INDEX idx_device_time_stamp (device_name, time_stamp)
    );
    """

    with get_db_connection() as conn:
        cursor = conn.cursor()
        cursor.execute(create_table_sql)
        conn.commit()
        cursor.close()

@asynccontextmanager
async def lifespan(_: FastAPI):
    initialize_database()
    yield

app = FastAPI(lifespan=lifespan)

@app.post("/receive")
def receive_counts(payload: DeviceCountPayload):
    insert_sql = f"""
    INSERT INTO {TABLE_NAME}
        (device_name, person_delta, vehicle_delta)
    VALUES
        (%s, %s, %s)
    """

    try:
        with get_db_connection() as conn:
            cursor = conn.cursor()

            cursor.execute(
                insert_sql,
                (
                    payload.device,
                    payload.person_delta,
                    payload.vehicle_delta,
                ),
            )

            conn.commit()
            inserted_id = cursor.lastrowid
            cursor.close()

        return {
            "ok": True,
            "id": inserted_id,
        }

    except mysql.connector.Error as err:
        raise HTTPException(
            status_code=501,
            detail=f"MySQL error: {err}",
        )