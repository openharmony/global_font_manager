/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import { BusinessError } from '@ohos.base';

export default namespace fontManager {
  loadLibraryWithPermissionCheck("fontManager_ani.z", "@ohos.fontManager");
  export interface DataMigrationProgress {
    timeRemaining: int;
    progressPercentage: int;
  }

  export class DataMigrationProgressImpl implements DataMigrationProgress {
    timeRemaining: int = 0;
    progressPercentage: int = 0;

    public constructor(timeRemaining: int, progressPercentage: int) {
      this.timeRemaining = timeRemaining;
      this.progressPercentage = progressPercentage;
    }
  }

  export interface DataMigrationCallback {
    onHeartBeat(): void;
    onProgress(progress: DataMigrationProgress): void;
    onResult(result: int): void;
  }

  native function nativeInstallFont(path: string): int;

  native function nativeUninstallFont(fullName: string): int;

  export native function dataMigration(callback: DataMigrationCallback): int;

  export function installFont(path: string): Promise<int> {
    return new Promise<int>((resolve, reject) => {
      taskpool.execute((): int => {
          return nativeInstallFont(path);
      }).then((content: Any) => {
          resolve(content as int);
      }, (err: Error): void => {
          reject(err);
      });
    });
  };
  

  export function uninstallFont(fullName: string): Promise<int> {
    return new Promise<int>((resolve, reject) => {
      taskpool.execute((): int => {
          return nativeUninstallFont(fullName);
      }).then((content: Any) => {
          resolve(content as int);
      }, (err: Error): void => {
          reject(err);
      });
    });
  };

}
