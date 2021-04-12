from django.urls import path
from moistureduino import views

urlpatterns = [
    path('entries/', views.entry_list),
    path('entries/<int:pk>/', views.entry_detail),
]


