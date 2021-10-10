from django.urls import path, include
from rest_framework.routers import DefaultRouter
from moistureduino import views

# Create a router and register our viewsets with it.
router = DefaultRouter()
router.register(r'alerts', views.AlertViewSet)
router.register(r'entries', views.EntryViewSet)
router.register(r'pumping_entries', views.PumpingEntryViewSet)
router.register(r'users', views.UserViewSet)

# The API URLs are now determined automatically by the router.
urlpatterns = [
    path('', include(router.urls)),
]
